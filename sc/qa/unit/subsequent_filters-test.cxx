/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2010 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

#include <editeng/brshitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/borderline.hxx>
#include <dbdata.hxx>
#include "validat.hxx"
#include "cell.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>

#define CALC_DEBUG_OUTPUT 0
#define TEST_BUG_FILES 0

#include "helper/qahelper.hxx"

#define ODS_FORMAT_TYPE 50331943
#define XLS_FORMAT_TYPE 318767171
#define XLSX_FORMAT_TYPE 268959811
#define CSV_FORMAT_TYPE  (SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_ALIEN | SFX_FILTER_USESOPTIONS)
#define HTML_FORMAT_TYPE (SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_ALIEN | SFX_FILTER_USESOPTIONS)

#define ODS     0
#define XLS     1
#define XLSX    2
#define CSV     3
#define HTML    4

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

struct FileFormat {
    const char* pName; const char* pFilterName; const char* pTypeName; sal_uLong nFormatType;
};

FileFormat aFileFormats[] = {
    { "ods" , "calc8", "", ODS_FORMAT_TYPE },
    { "xls" , "MS Excel 97", "calc_MS_EXCEL_97", XLS_FORMAT_TYPE },
    { "xlsx", "Calc MS Excel 2007 XML" , "MS Excel 2007 XML", XLSX_FORMAT_TYPE },
    { "csv" , "Text - txt - csv (StarCalc)", "generic_Text", CSV_FORMAT_TYPE },
    { "html" , "calc_HTML_WebQuery", "generic_HTML", HTML_FORMAT_TYPE }
};

}

/* Implementation of Filters test */

class ScFiltersTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    ScFiltersTest();

    virtual bool load(const rtl::OUString &rFilter, const rtl::OUString &rURL, const rtl::OUString &rUserData);
    ScDocShellRef load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
        const rtl::OUString &rUserData, const rtl::OUString& rTypeName, sal_uLong nFormatType=0);

    void createFileURL(const rtl::OUString& aFileBase, const rtl::OUString& aFileExtension, rtl::OUString& rFilePath);
    void createCSVPath(const rtl::OUString& aFileBase, rtl::OUString& rFilePath);

    virtual void setUp();
    virtual void tearDown();

    //ods, xls, xlsx filter tests
    void testRangeNameXLS();
    void testRangeNameXLSX();
    void testHardRecalcODS();
    void testFunctionsODS();
    void testCachedFormulaResultsODS();
    void testVolatileFunctionsODS();
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
    void testBordersOoo33();
    void testBugFixesODS();
    void testBugFixesXLS();
    void testBugFixesXLSX();
    void testBrokenQuotesCSV();
    void testMergedCellsODS();
    void testRepeatedColumnsODS();
    void testDataValidityODS();

    void testColorScale();
    void testDataBar();
    void testCondFormat();

    //change this test file only in excel and not in calc
    void testSharedFormulaXLSX();
    void testCellValueXLSX();

    //misc tests unrelated to the import filters
    void testPasswordNew();
    void testPasswordOld();

    //test shape import
    void testControlImport();

    void testNumberFormatHTML();
    void testNumberFormatCSV();

    void testCellAnchoredShapesODS();

    void testPivotTableBasicODS();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testRangeNameXLS);
    CPPUNIT_TEST(testRangeNameXLSX);
    CPPUNIT_TEST(testHardRecalcODS);
    CPPUNIT_TEST(testFunctionsODS);
    CPPUNIT_TEST(testCachedFormulaResultsODS);
    CPPUNIT_TEST(testVolatileFunctionsODS);
    CPPUNIT_TEST(testCachedMatrixFormulaResultsODS);
    CPPUNIT_TEST(testDatabaseRangesODS);
    CPPUNIT_TEST(testDatabaseRangesXLS);
    CPPUNIT_TEST(testDatabaseRangesXLSX);
    CPPUNIT_TEST(testFormatsODS);
    CPPUNIT_TEST(testFormatsXLS);
    CPPUNIT_TEST(testFormatsXLSX);
    CPPUNIT_TEST(testMatrixODS);
    CPPUNIT_TEST(testMatrixXLS);
    CPPUNIT_TEST(testBorderODS);
    CPPUNIT_TEST(testBorderXLS);
    CPPUNIT_TEST(testBordersOoo33);
    CPPUNIT_TEST(testBugFixesODS);
    CPPUNIT_TEST(testBugFixesXLS);
    CPPUNIT_TEST(testBugFixesXLSX);
    CPPUNIT_TEST(testMergedCellsODS);
    CPPUNIT_TEST(testRepeatedColumnsODS);
    CPPUNIT_TEST(testDataValidityODS);
    CPPUNIT_TEST(testBrokenQuotesCSV);
    CPPUNIT_TEST(testSharedFormulaXLSX);
    CPPUNIT_TEST(testCellValueXLSX);
    CPPUNIT_TEST(testControlImport);

    CPPUNIT_TEST(testColorScale);
    CPPUNIT_TEST(testDataBar);
    CPPUNIT_TEST(testCondFormat);

    CPPUNIT_TEST(testNumberFormatHTML);
    CPPUNIT_TEST(testNumberFormatCSV);

    CPPUNIT_TEST(testCellAnchoredShapesODS);

    CPPUNIT_TEST(testPivotTableBasicODS);

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
    CPPUNIT_TEST_SUITE_END();

private:
    void testPassword_Impl(const rtl::OUString& rFileNameBase);
    ScDocShellRef loadDoc(const rtl::OUString& rName, sal_Int32 nType);

    uno::Reference<uno::XInterface> m_xCalcComponent;
    ::rtl::OUString m_aBaseString;
};

ScDocShellRef ScFiltersTest::load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
    const rtl::OUString &rUserData, const rtl::OUString& rTypeName, sal_uLong nFormatType)
{
    sal_uInt32 nFormat = 0;
    if (nFormatType)
        nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;
    SfxFilter* aFilter = new SfxFilter(
        rFilter,
        rtl::OUString(), nFormatType, nFormat, rTypeName, 0, rtl::OUString(),
        rUserData, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/scalc*")) );
    aFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    ScDocShellRef xDocShRef = new ScDocShell;
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ);
    pSrcMed->SetFilter(aFilter);
    if (!xDocShRef->DoLoad(pSrcMed))
    {
        xDocShRef->DoClose();
        // load failed.
        xDocShRef.Clear();
    }

    return xDocShRef;
}

bool ScFiltersTest::load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
    const rtl::OUString &rUserData)
{
    ScDocShellRef xDocShRef = load(rFilter, rURL, rUserData, rtl::OUString());
    bool bLoaded = xDocShRef.Is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

ScDocShellRef ScFiltersTest::loadDoc(const rtl::OUString& rName, sal_Int32 nFormat)
{
    rtl::OUString aFileExtension(aFileFormats[nFormat].pName, strlen(aFileFormats[nFormat].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[nFormat].pFilterName, strlen(aFileFormats[nFormat].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL( rName, aFileExtension, aFileName );
    rtl::OUString aFilterType(aFileFormats[nFormat].pTypeName, strlen(aFileFormats[nFormat].pTypeName), RTL_TEXTENCODING_UTF8);
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[nFormat].nFormatType);
    CPPUNIT_ASSERT(xDocSh.Is());
    return xDocSh;
}

void ScFiltersTest::createFileURL(const rtl::OUString& aFileBase, const rtl::OUString& aFileExtension, rtl::OUString& rFilePath)
{
    rtl::OUString aSep(RTL_CONSTASCII_USTRINGPARAM("/"));
    rtl::OUStringBuffer aBuffer( getSrcRootURL() );
    aBuffer.append(m_aBaseString).append(aSep).append(aFileExtension);
    aBuffer.append(aSep).append(aFileBase).append(aFileExtension);
    rFilePath = aBuffer.makeStringAndClear();
}

void ScFiltersTest::createCSVPath(const rtl::OUString& aFileBase, rtl::OUString& rCSVPath)
{
    rtl::OUStringBuffer aBuffer(getSrcRootPath());
    aBuffer.append(m_aBaseString).append(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/contentCSV/")));
    aBuffer.append(aFileBase).append(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("csv")));
    rCSVPath = aBuffer.makeStringAndClear();
}

namespace {

void testRangeNameImpl(ScDocument* pDoc)
{
    //check one range data per sheet and one global more detailed
    //add some more checks here
    ScRangeData* pRangeData = pDoc->GetRangeName()->findByUpperName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GLOBAL1")));
    CPPUNIT_ASSERT_MESSAGE("range name Global1 not found", pRangeData);
    double aValue;
    pDoc->GetValue(1,0,0,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Global1 should reference Sheet1.A1", 1.0, aValue);
    pRangeData = pDoc->GetRangeName(0)->findByUpperName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LOCAL1")));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 not found", pRangeData);
    pDoc->GetValue(1,2,0,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet1.Local1 should reference Sheet1.A3", 3.0, aValue);
    pRangeData = pDoc->GetRangeName(1)->findByUpperName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LOCAL2")));
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
}

}

void ScFiltersTest::testRangeNameXLS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("named-ranges-global."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, XLS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testRangeNameImpl(pDoc);

    rtl::OUString aSheet2CSV(RTL_CONSTASCII_USTRINGPARAM("rangeExp_Sheet2."));
    rtl::OUString aCSVPath;
    createCSVPath( aSheet2CSV, aCSVPath );
    // fdo#44587
    testFile( aCSVPath, pDoc, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testRangeNameXLSX()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("named-ranges-global."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, XLSX);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testRangeNameImpl(pDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testHardRecalcODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("hard-recalc."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, ODS );
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load hard-recalc.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    rtl::OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("hard-recalc.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testFunctionsODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("functions."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, ODS );
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    rtl::OUString aCSVFileName;

    //test logical functions
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("logical-functions.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);
    //test spreadsheet functions
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("spreadsheet-functions.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 1);
    //test mathematical functions
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("mathematical-functions.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 2, PureString);
    //test informations functions
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("information-functions.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 3);

    xDocSh->DoClose();
}

void ScFiltersTest::testCachedFormulaResultsODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("functions."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, ODS );

    CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    rtl::OUString aCSVFileName;

    //test cached formula results of logical functions
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("logical-functions.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);
    //test cached formula results of spreadsheet functions
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("spreadsheet-functions.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 1);
    //test cached formula results of mathematical functions
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("mathematical-functions.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 2, PureString);
    //test cached formula results of informations functions
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("information-functions.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 3);

    xDocSh->DoClose();
}

void ScFiltersTest::testVolatileFunctionsODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("volatile."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, ODS );

    CPPUNIT_ASSERT_MESSAGE("Failed to load volatile.ods", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    //we want to me sure that volatile functions are always recalculated
    //regardless of cached results.  if you update the ods file, you must
    //update the values here.
    //if NOW() is recacluated, then it should never equal sTodayCache
    OUString sTodayCache("07/11/12 12:28 AM");
    OUString sTodayRecalc(pDoc->GetString(0,1,0));
    CPPUNIT_ASSERT(sTodayCache != sTodayRecalc);

    OUString sTodayRecalcRef(pDoc->GetString(2,1,0));
    CPPUNIT_ASSERT(sTodayCache != sTodayRecalcRef);

    xDocSh->DoClose();
}

void ScFiltersTest::testCachedMatrixFormulaResultsODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("matrix."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load matrix.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    //test matrix
    rtl::OUString aCSVFileName;
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("matrix.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);
    //test matrices with special cases
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("matrix2.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 1);
    //The above testFile() does not catch the below case.
    //If a matrix formula has a matrix reference cell that is intended to have
    //a blank text result, the matrix reference cell is actually saved(export)
    //as a float cell with 0 as the value and an empty <text:p/>.
    //Import works around this by setting these cells as text cells so that
    //the blank text is used for display instead of the number 0.
    //If this is working properly, the following cell should NOT have value data.
    CPPUNIT_ASSERT(!pDoc->GetCell(ScAddress(3,5,1))->HasValueData());

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
        rtl::OUString aString;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: A11: formula result is incorrect", 4.0, aValue);
        pDoc->GetValue(1, 10, 1, aValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: B11: formula result is incorrect", 2.0, aValue);
    }
    double aValue;
    pDoc->GetValue(3,10,1, aValue);
    rtl::OUString aString;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: D11: formula result is incorrect", 4.0, aValue);
    pDoc->GetValue(4, 10, 1, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: E11: formula result is incorrect", 2.0, aValue);

}

}

void ScFiltersTest::testDatabaseRangesODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("database."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 0);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testDBRanges_Impl(pDoc, ODS);
    xDocSh->DoClose();
}

void ScFiltersTest::testDatabaseRangesXLS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("database."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 1);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testDBRanges_Impl(pDoc, XLS);
    xDocSh->DoClose();
}

void ScFiltersTest::testDatabaseRangesXLSX()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("database."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 2);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testDBRanges_Impl(pDoc, XLSX);
    xDocSh->DoClose();
}

namespace {

void testFormats_Impl(ScFiltersTest* pFiltersTest, ScDocument* pDoc, sal_Int32 nFormat)
{
    //test Sheet1 with csv file
    rtl::OUString aCSVFileName;
    pFiltersTest->createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("numberFormat.")), aCSVFileName);
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
        rtl::OUString aTestStr;
        pDoc->GetString(3,0,1, aTestStr);
        rtl::OUString aKnownGoodStr("text14 space");
        CPPUNIT_ASSERT_EQUAL( aKnownGoodStr, aTestStr );
        //test case for cell text with line breaks.
        pDoc->GetString(3,5,1, aTestStr);
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
    if ( nFormat == ODS )
    {
        rtl::OUString aCondString = getConditionalFormatString(pDoc, 3,0,2);
        pFiltersTest->createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("conditionalFormatting.")), aCSVFileName);
        testCondFile(aCSVFileName, pDoc, 2);
    }
}

}

void ScFiltersTest::testFormatsODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("formats."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 0);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testFormats_Impl(this, pDoc, ODS);
    xDocSh->DoClose();
}

void ScFiltersTest::testFormatsXLS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("formats."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 1);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testFormats_Impl(this, pDoc, XLS);
    xDocSh->DoClose();
}

void ScFiltersTest::testFormatsXLSX()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("formats."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 2);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testFormats_Impl(this, pDoc, XLSX);
    xDocSh->DoClose();
}

void ScFiltersTest::testMatrixODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("matrix."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, 0);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    rtl::OUString aCSVFileName;
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("matrix.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testMatrixXLS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("matrix."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, 1);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load matrix.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    rtl::OUString aCSVFileName;
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("matrix.")), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testBorderODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("border."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, 0);

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

void ScFiltersTest::testBorderXLS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("border."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, 1);

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
    CPPUNIT_ASSERT_EQUAL(pRight->GetWidth(),4L);

    pDoc->GetBorderLines( 3, 5, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(pRight->GetBorderLineStyle(),
            table::BorderLineStyle::SOLID);
    CPPUNIT_ASSERT_EQUAL(pRight->GetWidth(),12L);

    pDoc->GetBorderLines( 5, 7, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(pRight->GetBorderLineStyle(),
            table::BorderLineStyle::SOLID);
    CPPUNIT_ASSERT_EQUAL(pRight->GetWidth(),16L);
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

    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("borders_ooo33."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, 0);

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
}

void ScFiltersTest::testBugFixesODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("bug-fixes."));
    rtl::OUString aFileExtension(aFileFormats[0].pName, strlen(aFileFormats[0].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[0].pFilterName, strlen(aFileFormats[0].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    rtl::OUString aFilterType(aFileFormats[0].pTypeName, strlen(aFileFormats[0].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << aFileFormats[0].pName << " Test" << std::endl;
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[0].nFormatType);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.ods", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    {
        // fdo
        rtl::OUString aCSVFileName;
        createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bugFix_Sheet2.")), aCSVFileName);
        testFile(aCSVFileName, pDoc, 1);
    }

    {
        // fdo#40426
        ScDBData* pDBData = pDoc->GetDBCollection()->getNamedDBs().findByName("DBRange1");
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(pDBData->HasHeader());
        // no header
        pDBData = pDoc->GetDBCollection()->getNamedDBs().findByName("DBRange2");
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(!pDBData->HasHeader());
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testBugFixesXLS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("bug-fixes."));
    rtl::OUString aFileExtension(aFileFormats[1].pName, strlen(aFileFormats[1].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[1].pFilterName, strlen(aFileFormats[1].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    rtl::OUString aFilterType(aFileFormats[1].pTypeName, strlen(aFileFormats[1].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << aFileFormats[1].pName << " Test" << std::endl;
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[1].nFormatType);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.xls", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); //remove with first test
    xDocSh->DoClose();
}

void ScFiltersTest::testBugFixesXLSX()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("bug-fixes."));
    rtl::OUString aFileExtension(aFileFormats[2].pName, strlen(aFileFormats[2].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[2].pFilterName, strlen(aFileFormats[2].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    rtl::OUString aFilterType(aFileFormats[2].pTypeName, strlen(aFileFormats[2].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << aFileFormats[2].pName << " Test" << std::endl;
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[2].nFormatType);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.xlsx", xDocSh.Is());
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
    rtl::OString sTab = rtl::OString::valueOf( static_cast<sal_Int32>(rStartAddress.Tab() + 1) );
    rtl::OString msg = "Merged cells are not correctly imported on sheet" + sTab;
    rtl::OString msgCol = msg + "; end col";
    rtl::OString msgRow = msg + "; end row";
    CPPUNIT_ASSERT_EQUAL_MESSAGE( msgCol.pData->buffer, rExpectedEndAddress.Col(), nActualEndCol );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( msgRow.pData->buffer, rExpectedEndAddress.Row(), nActualEndRow );
}

}

void ScFiltersTest::testMergedCellsODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("merged."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, 0);

    ScDocument* pDoc = xDocSh->GetDocument();

    //check sheet1 content
    rtl::OUString aCSVFileName1;
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("merged1.")), aCSVFileName1);
    testFile(aCSVFileName1, pDoc, 0);

    //check sheet1 merged cells
    checkMergedCells( pDoc, ScAddress( 0, 0, 0 ),  ScAddress( 5, 11, 0 ) );
    checkMergedCells( pDoc, ScAddress( 7, 2, 0 ),  ScAddress( 9, 12, 0 ) );
    checkMergedCells( pDoc, ScAddress( 3, 15, 0 ),  ScAddress( 7, 23, 0 ) );

    //check sheet2 content
    rtl::OUString aCSVFileName2;
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("merged2.")), aCSVFileName2);
    testFile(aCSVFileName2, pDoc, 1);

    //check sheet2 merged cells
    checkMergedCells( pDoc, ScAddress( 4, 3, 1 ),  ScAddress( 6, 15, 1 ) );

    xDocSh->DoClose();
}

void ScFiltersTest::testRepeatedColumnsODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("repeatedColumns."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, 0);

    ScDocument* pDoc = xDocSh->GetDocument();

    //text
    rtl::OUString aCSVFileName1;
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("repeatedColumns1.")), aCSVFileName1);
    testFile(aCSVFileName1, pDoc, 0);

    //numbers
    rtl::OUString aCSVFileName2;
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("repeatedColumns2.")), aCSVFileName2);
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
    rtl::OStringBuffer sMsg("Data Validation Entry with base-cell-address: (");
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
                rtl::OStringBuffer sMsg("\nData validation entry base-cell-address: (");
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
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("dataValidity."));
    ScDocShellRef xDocSh = loadDoc( aFileNameBase, 0);

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
    rtl::OUString aCSVFileName1;
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("dataValidity1.")), aCSVFileName1);
    testFile(aCSVFileName1, pDoc, 0);

    rtl::OUString aCSVFileName2;
    createCSVPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("dataValidity2.")), aCSVFileName2);
    testFile(aCSVFileName2, pDoc, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testBrokenQuotesCSV()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("fdo48621_broken_quotes."));
    rtl::OUString aFileExtension(aFileFormats[CSV].pName, strlen(aFileFormats[CSV].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[CSV].pFilterName, strlen(aFileFormats[CSV].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    rtl::OUString aFilterType(aFileFormats[CSV].pTypeName, strlen(aFileFormats[CSV].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << aFileFormats[CSV].pName << " Test" << std::endl;
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[CSV].nFormatType);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo48621_broken_quotes.csv", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); //remove with first test

    rtl::OUString aSheet2CSV(RTL_CONSTASCII_USTRINGPARAM("fdo48621_broken_quotes_exported."));
    rtl::OUString aCSVPath;
    createCSVPath( aSheet2CSV, aCSVPath );
    // fdo#48621
    testFile( aCSVPath, pDoc, 0, PureString);

    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaXLSX()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("shared-formula."));
    rtl::OUString aFileExtension(aFileFormats[XLSX].pName, strlen(aFileFormats[XLSX].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[XLSX].pFilterName, strlen(aFileFormats[XLSX].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    rtl::OUString aFilterType(aFileFormats[XLSX].pTypeName, strlen(aFileFormats[XLSX].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << aFileFormats[XLSX].pName << " Test" << std::endl;
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[XLSX].nFormatType);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load shared-formula.xlsx", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); //remove with first test

    rtl::OUString aCSVPath;
    createCSVPath( aFileNameBase, aCSVPath );
    testFile( aCSVPath, pDoc, 0 );

    //test some additional properties
    ScRangeName* pName = pDoc->GetRangeName();
    for (ScRangeName::iterator itr = pName->begin(); itr != pName->end(); ++itr)
    {
        CPPUNIT_ASSERT(itr->second->GetType() & RT_SHARED);
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testCellValueXLSX()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("cell-value."));
    rtl::OUString aFileExtension(aFileFormats[XLSX].pName, strlen(aFileFormats[XLSX].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[XLSX].pFilterName, strlen(aFileFormats[XLSX].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    rtl::OUString aFilterType(aFileFormats[XLSX].pTypeName, strlen(aFileFormats[XLSX].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << aFileFormats[XLSX].pName << " Test" << std::endl;
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[XLSX].nFormatType);

    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-value.xlsx", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); //remove with first test

    rtl::OUString aCSVPath;
    createCSVPath( aFileNameBase, aCSVPath );
    testFile( aCSVPath, pDoc, 0 );

    xDocSh->DoClose();
}

void ScFiltersTest::testPassword_Impl(const rtl::OUString& aFileNameBase)
{
    rtl::OUString aFileExtension(aFileFormats[0].pName, strlen(aFileFormats[0].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[0].pFilterName, strlen(aFileFormats[0].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    rtl::OUString aFilterType(aFileFormats[0].pTypeName, strlen(aFileFormats[0].pTypeName), RTL_TEXTENCODING_UTF8);

    sal_uInt32 nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;
    SfxFilter* aFilter = new SfxFilter(
        aFilterName,
        rtl::OUString(), aFileFormats[0].nFormatType, nFormat, aFilterType, 0, rtl::OUString(),
        rtl::OUString(), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/scalc*")) );
    aFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    ScDocShellRef xDocSh = new ScDocShell;
    SfxMedium* pMedium = new SfxMedium(aFileName, STREAM_STD_READWRITE);
    SfxItemSet* pSet = pMedium->GetItemSet();
    pSet->Put(SfxStringItem(SID_PASSWORD, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test"))));
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
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("password."));
    testPassword_Impl(aFileNameBase);
}

void ScFiltersTest::testPasswordOld()
{
    //tests opening a file with old password algorithm
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("passwordOld."));
    testPassword_Impl(aFileNameBase);
}

void ScFiltersTest::testControlImport()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("singlecontrol."));
    rtl::OUString aFileExtension(aFileFormats[XLSX].pName, strlen(aFileFormats[XLSX].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[XLSX].pFilterName, strlen(aFileFormats[XLSX].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    rtl::OUString aFilterType(aFileFormats[XLSX].pTypeName, strlen(aFileFormats[XLSX].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << aFileFormats[XLSX].pName << " Test" << std::endl;
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[XLSX].nFormatType);

    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-value.xlsx", xDocSh.Is());

    uno::Reference< frame::XModel > xModel = xDocSh->GetModel();
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xModel, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xIA->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA_DrawPage(xDrawPageSupplier->getDrawPage(), UNO_QUERY_THROW);
    uno::Reference< drawing::XControlShape > xControlShape(xIA_DrawPage->getByIndex(0), UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xControlShape.is());
    xDocSh->DoClose();
}

void ScFiltersTest::testNumberFormatHTML()
{
    OUString aFileNameBase("numberformat.");
    OUString aFileExt = OUString::createFromAscii(aFileFormats[HTML].pName);
    OUString aFilterName = OUString::createFromAscii(aFileFormats[HTML].pFilterName);
    OUString aFilterType = OUString::createFromAscii(aFileFormats[HTML].pTypeName);

    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExt, aFileName);
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[HTML].nFormatType);
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
    OUString aFileNameBase("numberformat.");
    OUString aFileExt = OUString::createFromAscii(aFileFormats[CSV].pName);
    OUString aFilterName = OUString::createFromAscii(aFileFormats[CSV].pFilterName);
    OUString aFilterType = OUString::createFromAscii(aFileFormats[CSV].pTypeName);

    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExt, aFileName);
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[CSV].nFormatType);
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

void ScFiltersTest::testCellAnchoredShapesODS()
{
    OUString aFileNameBase("cell-anchored-shapes.");
    OUString aFileExt = OUString::createFromAscii(aFileFormats[ODS].pName);
    OUString aFilterName = OUString::createFromAscii(aFileFormats[ODS].pFilterName);
    OUString aFilterType = OUString::createFromAscii(aFileFormats[ODS].pTypeName);

    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExt, aFileName);
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[ODS].nFormatType);
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
    OUString aFileNameBase("pivot-table-basic.");
    OUString aFileExt = OUString::createFromAscii(aFileFormats[ODS].pName);
    OUString aFilterName = OUString::createFromAscii(aFileFormats[ODS].pFilterName);
    OUString aFilterType = OUString::createFromAscii(aFileFormats[ODS].pTypeName);

    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExt, aFileName);
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[ODS].nFormatType);
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

void ScFiltersTest::testColorScale()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("colorScale."));
    rtl::OUString aFileExtension(aFileFormats[ODS].pName, strlen(aFileFormats[ODS].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[ODS].pFilterName, strlen(aFileFormats[ODS].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    rtl::OUString aFilterType(aFileFormats[ODS].pTypeName, strlen(aFileFormats[ODS].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << aFileFormats[ODS].pName << " Test" << std::endl;
    ScDocShellRef xDocSh = load (aFilterName, aFileName, rtl::OUString(), aFilterType, aFileFormats[ODS].nFormatType);

    CPPUNIT_ASSERT_MESSAGE("Failed to load colorScale.ods", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();

    const ScPatternAttr* pPattern = pDoc->GetPattern(1,1,0);
    sal_uLong nIndex = static_cast<const SfxUInt32Item&>(pPattern->GetItem(ATTR_CONDITIONAL)).GetValue();
    CPPUNIT_ASSERT(nIndex);
    ScConditionalFormatList* pCondFormatList = pDoc->GetCondFormList(0);
    const ScConditionalFormat* pFormat = pCondFormatList->GetFormat(nIndex);
    CPPUNIT_ASSERT(pFormat);
    pFormat->dumpInfo();

    pPattern = pDoc->GetPattern(1,5,0);
    nIndex = static_cast<const SfxUInt32Item&>(pPattern->GetItem(ATTR_CONDITIONAL)).GetValue();
    CPPUNIT_ASSERT(nIndex);
    pFormat = pCondFormatList->GetFormat(nIndex);
    CPPUNIT_ASSERT(pFormat);
}

void ScFiltersTest::testDataBar()
{

}

void ScFiltersTest::testCondFormat()
{

}

ScFiltersTest::ScFiltersTest()
      : m_aBaseString(RTL_CONSTASCII_USTRINGPARAM("/sc/qa/unit/data"))
{
}

void ScFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.SpreadsheetDocument")));
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
