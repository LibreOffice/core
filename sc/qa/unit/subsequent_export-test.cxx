/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>

#include "helper/qahelper.hxx"
#include "helper/shared_test_impl.hxx"

#include "docsh.hxx"
#include "postit.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"
#include "cellform.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"

#include "svx/svdoole2.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScExportTest : public ScBootstrapFixture
{
public:
    ScExportTest();

    virtual void setUp();
    virtual void tearDown();

    ScDocShellRef saveAndReloadPassword( ScDocShell*, const OUString&, const OUString&, const OUString&, sal_uLong );

    void test();
    void testPasswordExport();
    void testConditionalFormatExportODS();
    void testConditionalFormatExportXLSX();
    void testColorScaleExportODS();
    void testColorScaleExportXLSX();
    void testDataBarExportODS();
    void testDataBarExportXLSX();
    void testMiscRowHeightExport();
    void testNamedRangeBugfdo62729();

    void testInlineArrayXLS();
    void testEmbeddedChartXLS();
    void testFormulaReferenceXLS();

    CPPUNIT_TEST_SUITE(ScExportTest);
    CPPUNIT_TEST(test);
#if !defined(MACOSX) && !defined(DRAGONFLY)
    CPPUNIT_TEST(testPasswordExport);
#endif
    CPPUNIT_TEST(testConditionalFormatExportODS);
    CPPUNIT_TEST(testConditionalFormatExportXLSX);
    CPPUNIT_TEST(testColorScaleExportODS);
    CPPUNIT_TEST(testColorScaleExportXLSX);
    CPPUNIT_TEST(testMiscRowHeightExport);
    CPPUNIT_TEST(testNamedRangeBugfdo62729);
    CPPUNIT_TEST(testInlineArrayXLS);
    CPPUNIT_TEST(testEmbeddedChartXLS);
    CPPUNIT_TEST(testFormulaReferenceXLS);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;

};

ScDocShellRef ScExportTest::saveAndReloadPassword(ScDocShell* pShell, const OUString &rFilter,
    const OUString &rUserData, const OUString& rTypeName, sal_uLong nFormatType)
{
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    SfxMedium aStoreMedium( aTempFile.GetURL(), STREAM_STD_WRITE );
    sal_uInt32 nExportFormat = 0;
    if (nFormatType == ODS_FORMAT_TYPE)
        nExportFormat = SFX_FILTER_EXPORT | SFX_FILTER_USESOPTIONS;
    SfxFilter* pExportFilter = new SfxFilter(
        rFilter,
        OUString(), nFormatType, nExportFormat, rTypeName, 0, OUString(),
        rUserData, OUString("private:factory/scalc*") );
    pExportFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
    aStoreMedium.SetFilter(pExportFilter);
    SfxItemSet* pExportSet = aStoreMedium.GetItemSet();
    uno::Sequence< beans::NamedValue > aEncryptionData = comphelper::OStorageHelper::CreatePackageEncryptionData( OUString("test") );
    uno::Any xEncryptionData;
    xEncryptionData <<= aEncryptionData;
    pExportSet->Put(SfxUnoAnyItem(SID_ENCRYPTIONDATA, xEncryptionData));

    uno::Reference< embed::XStorage > xMedStorage = aStoreMedium.GetStorage();
    ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( xMedStorage, aEncryptionData );

    pShell->DoSaveAs( aStoreMedium );
    pShell->DoClose();

    //std::cout << "File: " << aTempFile.GetURL() << std::endl;

    sal_uInt32 nFormat = 0;
    if (nFormatType == ODS_FORMAT_TYPE)
        nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;

    OUString aPass("test");
    return load(aTempFile.GetURL(), rFilter, rUserData, rTypeName, nFormatType, nFormat, SOFFICE_FILEFORMAT_CURRENT, &aPass);
}

void ScExportTest::test()
{
    ScDocShell* pShell = new ScDocShell(
        SFXMODEL_STANDARD |
        SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
        SFXMODEL_DISABLE_DOCUMENT_RECOVERY);
    pShell->DoInitNew();

    ScDocument* pDoc = pShell->GetDocument();

    pDoc->SetValue(0,0,0, 1.0);
    CPPUNIT_ASSERT(pDoc);

    ScDocShellRef xDocSh = saveAndReload( pShell, ODS );

    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pLoadedDoc = xDocSh->GetDocument();
    double aVal = pLoadedDoc->GetValue(0,0,0);
    ASSERT_DOUBLES_EQUAL(aVal, 1.0);
}

void ScExportTest::testPasswordExport()
{
    ScDocShell* pShell = new ScDocShell(
        SFXMODEL_STANDARD |
        SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
        SFXMODEL_DISABLE_DOCUMENT_RECOVERY);
    pShell->DoInitNew();

    ScDocument* pDoc = pShell->GetDocument();

    pDoc->SetValue(0,0,0, 1.0);
    CPPUNIT_ASSERT(pDoc);

    sal_Int32 nFormat = ODS;
    OUString aFilterName(getFileFormats()[nFormat].pFilterName, strlen(getFileFormats()[nFormat].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFilterType(getFileFormats()[nFormat].pTypeName, strlen(getFileFormats()[nFormat].pTypeName), RTL_TEXTENCODING_UTF8);
    ScDocShellRef xDocSh = saveAndReloadPassword(pShell, aFilterName, OUString(), aFilterType, getFileFormats()[nFormat].nFormatType);

    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pLoadedDoc = xDocSh->GetDocument();
    double aVal = pLoadedDoc->GetValue(0,0,0);
    ASSERT_DOUBLES_EQUAL(aVal, 1.0);

    xDocSh->DoClose();
}

void ScExportTest::testConditionalFormatExportODS()
{
    ScDocShellRef xShell = loadDoc("new_cond_format_test.", ODS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), ODS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    OUString aCSVFile("new_cond_format_test.");
    OUString aCSVPath;
    createCSVPath( aCSVFile, aCSVPath );
    testCondFile(aCSVPath, pDoc, 0);

    xDocSh->DoClose();
}

void ScExportTest::testConditionalFormatExportXLSX()
{
    ScDocShellRef xShell = loadDoc("new_cond_format_test.", XLSX);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    OUString aCSVFile("new_cond_format_test.");
    OUString aCSVPath;
    createCSVPath( aCSVFile, aCSVPath );
    testCondFile(aCSVPath, pDoc, 0);

    xDocSh->DoClose();
}

void ScExportTest::testColorScaleExportODS()
{
    ScDocShellRef xShell = loadDoc("colorscale.", ODS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, ODS);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    testColorScale2Entry_Impl(pDoc);
    testColorScale3Entry_Impl(pDoc);

    xDocSh->DoClose();
}

void ScExportTest::testColorScaleExportXLSX()
{
    ScDocShellRef xShell = loadDoc("colorscale.", XLSX);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    testColorScale2Entry_Impl(pDoc);
    testColorScale3Entry_Impl(pDoc);

    xDocSh->DoClose();
}

void ScExportTest::testDataBarExportODS()
{
    ScDocShellRef xShell = loadDoc("databar.", ODS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, ODS);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    testDataBar_Impl(pDoc);

    xDocSh->DoClose();
}

void ScExportTest::testDataBarExportXLSX()
{
    ScDocShellRef xShell = loadDoc("databar.", XLSX);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    testDataBar_Impl(pDoc);

    xDocSh->DoClose();
}

void ScExportTest::testMiscRowHeightExport()
{
    TestParam::RowData DfltRowData[] =
    {
        { 0, 4, 0, 529, 0, false },
        { 5, 10, 0, 1058, 0, false },
        { 17, 20, 0, 1767, 0, false },
        // check last couple of row in document to ensure
        // they are 5.29mm ( effective default row xlsx height )
        { 1048573, 1048575, 0, 529, 0, false },
    };

    TestParam::RowData EmptyRepeatRowData[] =
    {
        // rows 0-4, 5-10, 17-20 are all set at various
        // heights, there is no content in the rows, there
        // was a bug where only the first row ( of repeated rows )
        // was set after export
        { 0, 4, 0, 529, 0, false },
        { 5, 10, 0, 1058, 0, false },
        { 17, 20, 0, 1767, 0, false },
    };

    TestParam aTestValues[] =
    {
        // Checks that some distributed ( non-empty ) heights remain set after export (roundtrip)
        // additionally there is effectively a default row height ( 5.29 mm ). So we test the
        // unset rows at the end of the document to ensure the effective xlsx default height
        // is set there too.
        { "miscrowheights.", XLSX, XLSX, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        // Checks that some distributed ( non-empty ) heights remain set after export (to xls)
        { "miscrowheights.", XLSX, XLS, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        // Checks that repreated rows ( of various heights ) remain set after export ( to xlsx )
        { "miscemptyrepeatedrowheights.", ODS, XLSX, SAL_N_ELEMENTS(EmptyRepeatRowData), EmptyRepeatRowData },
        // Checks that repreated rows ( of various heights ) remain set after export ( to xls )
        { "miscemptyrepeatedrowheights.", ODS, XLS, SAL_N_ELEMENTS(EmptyRepeatRowData), EmptyRepeatRowData },
    };
    miscRowHeightsTest( aTestValues, SAL_N_ELEMENTS(aTestValues) );
}


void ScExportTest::testNamedRangeBugfdo62729()
{
    ScDocShellRef xShell = loadDoc("fdo62729.", ODS);
    CPPUNIT_ASSERT(xShell.Is());
    ScDocument* pDoc = xShell->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    ScRangeName* pNames = pDoc->GetRangeName();
    //should be just a single named range
    CPPUNIT_ASSERT(pNames->size() == 1 );
    pDoc->DeleteTab(0);
    //should be still a single named range
    CPPUNIT_ASSERT(pNames->size() == 1 );
    ScDocShellRef xDocSh = saveAndReload(xShell, ODS);
    xShell->DoClose();

    CPPUNIT_ASSERT(xDocSh.Is());
    pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    pNames = pDoc->GetRangeName();
    //after reload should still have a named range
    CPPUNIT_ASSERT(pNames->size() == 1 );

    xDocSh->DoClose();
}

namespace {

void checkMatrixRange(ScDocument& rDoc, const ScRange& rRange)
{
    ScRange aMatRange;
    ScAddress aMatOrigin;
    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
        {
            ScAddress aPos(nCol, nRow, rRange.aStart.Tab());
            bool bIsMatrix = rDoc.GetMatrixFormulaRange(aPos, aMatRange);
            CPPUNIT_ASSERT_MESSAGE("Matrix expected, but not found.", bIsMatrix);
            CPPUNIT_ASSERT_MESSAGE("Wrong matrix range.", rRange == aMatRange);
            const ScFormulaCell* pCell = rDoc.GetFormulaCell(aPos);
            CPPUNIT_ASSERT_MESSAGE("This must be a formula cell.", pCell);

            bIsMatrix = pCell->GetMatrixOrigin(aMatOrigin);
            CPPUNIT_ASSERT_MESSAGE("Not a part of matrix formula.", bIsMatrix);
            CPPUNIT_ASSERT_MESSAGE("Wrong matrix origin.", aMatOrigin == aMatRange.aStart);
        }
    }
}

}

void ScExportTest::testInlineArrayXLS()
{
    ScDocShellRef xShell = loadDoc("inline-array.", XLS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // B2:C3 contains a matrix.
    checkMatrixRange(*pDoc, ScRange(1,1,0,2,2,0));

    // B5:D6 contains a matrix.
    checkMatrixRange(*pDoc, ScRange(1,4,0,3,5,0));

    // B8:C10 as well.
    checkMatrixRange(*pDoc, ScRange(1,7,0,2,9,0));

    xDocSh->DoClose();
}

void ScExportTest::testEmbeddedChartXLS()
{
    ScDocShellRef xShell = loadDoc("embedded-chart.", XLS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Make sure the 2nd sheet is named 'Chart1'.
    OUString aName;
    pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Chart1"), aName);

    const SdrOle2Obj* pOleObj = getSingleChartObject(*pDoc, 1);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    ScRangeList aRanges = getChartRanges(*pDoc, *pOleObj);
    CPPUNIT_ASSERT_MESSAGE("Label range (B3:B5) not found.", aRanges.In(ScRange(1,2,1,1,4,1)));
    CPPUNIT_ASSERT_MESSAGE("Data label (C2) not found.", aRanges.In(ScAddress(2,1,1)));
    CPPUNIT_ASSERT_MESSAGE("Data range (C3:C5) not found.", aRanges.In(ScRange(2,2,1,2,4,1)));

    xDocSh->DoClose();
}

void ScExportTest::testFormulaReferenceXLS()
{
    ScDocShellRef xShell = loadDoc("formula-reference.", XLS);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(xShell, XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    if (!checkFormula(*pDoc, ScAddress(3,1,0), "$A$2+$B$2+$C$2"))
        CPPUNIT_FAIL("Wrong formula in D2");

    if (!checkFormula(*pDoc, ScAddress(3,2,0), "A3+B3+C3"))
        CPPUNIT_FAIL("Wrong formula in D3");

    if (!checkFormula(*pDoc, ScAddress(3,5,0), "SUM($A$6:$C$6)"))
        CPPUNIT_FAIL("Wrong formula in D6");

    if (!checkFormula(*pDoc, ScAddress(3,6,0), "SUM(A7:C7)"))
        CPPUNIT_FAIL("Wrong formula in D7");

    if (!checkFormula(*pDoc, ScAddress(3,9,0), "$Two.$A$2+$Two.$B$2+$Two.$C$2"))
        CPPUNIT_FAIL("Wrong formula in D10");

    if (!checkFormula(*pDoc, ScAddress(3,10,0), "$Two.A3+$Two.B3+$Two.C3"))
        CPPUNIT_FAIL("Wrong formula in D11");

    if (!checkFormula(*pDoc, ScAddress(3,13,0), "MIN($Two.$A$2:$C$2)"))
        CPPUNIT_FAIL("Wrong formula in D14");

    if (!checkFormula(*pDoc, ScAddress(3,14,0), "MAX($Two.A3:C3)"))
        CPPUNIT_FAIL("Wrong formula in D15");

    xDocSh->DoClose();
}

ScExportTest::ScExportTest()
      : ScBootstrapFixture("/sc/qa/unit/data")
{
}

void ScExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScExportTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
