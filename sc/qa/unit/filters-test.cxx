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

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <inputopt.hxx>
#include <postit.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <userdat.hxx>
#include <formulacell.hxx>
#include <tabprotection.hxx>
#include <testlotus.hxx>
#include <dbdocfun.hxx>
#include <globalnames.hxx>
#include <dbdata.hxx>
#include <sortparam.hxx>
#include <scerrors.hxx>
#include <scopetools.hxx>
#include <scmod.hxx>
#include <undomanager.hxx>

#include <svx/svdocapt.hxx>
#include <svx/svdpage.hxx>
#include <tools/stream.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Filters test */

class ScFiltersTest
    : public test::FiltersTest
    , public ScBootstrapFixture
{
public:
    ScFiltersTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    virtual bool load( const OUString &rFilter, const OUString &rURL,
        const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion) override;
    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    //ods, xls, xlsx filter tests
    void testContentofz9704();
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
    void testSortWithSheetExternalReferencesODS_Impl( ScDocShellRef const & xDocShRef, SCROW nRow1, SCROW nRow2,
            bool bCheckRelativeInSheet );
    void testSortWithFormattingXLS();
    void testTooManyColsRows();
    void testForcepoint107();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testContentofz9704);
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
    CPPUNIT_TEST(testTooManyColsRows);
    CPPUNIT_TEST(testForcepoint107);

    CPPUNIT_TEST_SUITE_END();

private:
    bool mbUpdateReferenceOnSort; ///< Remember the configuration option so that we can set it back.
};

bool ScFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, SfxFilterFlags nFilterFlags,
    SotClipboardFormatId nClipboardID, unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load(rURL, rFilter, rUserData,
        OUString(), nFilterFlags, nClipboardID, nFilterVersion );
    bool bLoaded = xDocShRef.is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

void ScFiltersTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir("Quattro Pro 6.0",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/qpro/"));

    //warning, the current "sylk filter" in sc (docsh.cxx) automatically
    //chains on failure on trying as csv, rtf, etc. so "success" may
    //not indicate that it imported as .slk.
    testDir("SYLK",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/slk/"));

    testDir("MS Excel 97",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/xls/"));

    testDir("Calc Office Open XML",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/xlsx/"), OUString(), XLSX_FORMAT_TYPE);

    testDir("Calc Office Open XML",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/xlsm/"), OUString(), XLSX_FORMAT_TYPE);

    testDir("dBase",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/dbf/"));

    testDir("Lotus",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/wks/"));

#endif
}


void ScFiltersTest::testContentofz9704()
{
    OUString aFileName;
    createFileURL(u"ofz9704.", u"123", aFileName);
    SvFileStream aFileStream(aFileName, StreamMode::READ);
    TestImportWKS(aFileStream);
}

void ScFiltersTest::testSharedFormulaXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"shared-formula/basic.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();
    xDocSh->DoHardRecalc();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 18; ++i)
    {
        double fVal = rDoc.GetValue(ScAddress(1,i,0));
        double fCheck = i*10.0;
        CPPUNIT_ASSERT_EQUAL(fCheck, fVal);
    }

    ScFormulaCell* pCell = rDoc.GetFormulaCell(ScAddress(1,18,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pCell);
    ScFormulaCellGroupRef xGroup = pCell->GetCellGroup();
    CPPUNIT_ASSERT_MESSAGE("This cell should be a part of a cell group.", xGroup);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(1), xGroup->mpTopCell->aPos.Row());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(18), xGroup->mnLength);

    xDocSh->DoClose();

    // The following file contains shared formula whose range is inaccurate.
    // Excel can easily mess up shared formula ranges, so we need to be able
    // to handle these wrong ranges that Excel stores.

    xDocSh = loadDoc(u"shared-formula/gap.", FORMAT_XLS);
    ScDocument& rDoc2 = xDocSh->GetDocument();
    rDoc2.CalcAll();

    ASSERT_FORMULA_EQUAL(rDoc2, ScAddress(1,0,0), "A1*20", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(rDoc2, ScAddress(1,1,0), "A2*20", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(rDoc2, ScAddress(1,2,0), "A3*20", "Wrong formula.");

    // There is an intentional gap at row 4.

    ASSERT_FORMULA_EQUAL(rDoc2, ScAddress(1,4,0), "A5*20", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(rDoc2, ScAddress(1,5,0), "A6*20", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(rDoc2, ScAddress(1,6,0), "A7*20", "Wrong formula.");
    ASSERT_FORMULA_EQUAL(rDoc2, ScAddress(1,7,0), "A8*20", "Wrong formula.");

    // We re-group formula cells on load. Let's check that as well.

    ScFormulaCell* pFC = rDoc2.GetFormulaCell(ScAddress(1,0,0));
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This should be the top cell in formula group.", pFC->IsSharedTop());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());

    pFC = rDoc2.GetFormulaCell(ScAddress(1,4,0));
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This should be the top cell in formula group.", pFC->IsSharedTop());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());

    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"shared-formula/basic.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();
    xDocSh->DoHardRecalc();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 18; ++i)
    {
        double fVal = rDoc.GetValue(ScAddress(1,i,0));
        double fCheck = i*10.0;
        CPPUNIT_ASSERT_EQUAL(fCheck, fVal);
    }

    ScFormulaCell* pCell = rDoc.GetFormulaCell(ScAddress(1,18,0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pCell);
    ScFormulaCellGroupRef xGroup = pCell->GetCellGroup();
    CPPUNIT_ASSERT_MESSAGE("This cell should be a part of a cell group.", xGroup);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(1), xGroup->mpTopCell->aPos.Row());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(18), xGroup->mnLength);

    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaRefUpdateXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"shared-formula/refupdate.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();
    sc::AutoCalcSwitch aACSwitch(rDoc, true); // turn auto calc on.
    rDoc.DeleteRow(ScRange(0, 4, 0, rDoc.MaxCol(), 4, 0)); // delete row 5.

    struct TestCase {
        ScAddress aPos;
        const char* pExpectedFormula;
        const char* pErrorMsg;
    };

    TestCase aCases[4] = {
        { ScAddress(1, 0, 0),  "B29+1", "Wrong formula in B1" },
        { ScAddress(2, 0, 0),  "C29+1", "Wrong formula in C1" },
        { ScAddress(3, 0, 0),  "D29+1", "Wrong formula in D1" },
        { ScAddress(4, 0, 0),  "E29+1", "Wrong formula in E1" },
    };

    for (size_t nIdx = 0; nIdx < 4; ++nIdx)
    {
        TestCase& rCase = aCases[nIdx];
        ASSERT_FORMULA_EQUAL(rDoc, rCase.aPos, rCase.pExpectedFormula, rCase.pErrorMsg);
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testSheetNamesXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"sheet-names.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();

    std::vector<OUString> aTabNames = rDoc.GetAllTableNames();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The document should have 5 sheets in total.", size_t(5), aTabNames.size());
    CPPUNIT_ASSERT_EQUAL(OUString("S&P"), aTabNames[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sam's Club"), aTabNames[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("\"The Sheet\""), aTabNames[2]);
    CPPUNIT_ASSERT_EQUAL(OUString("A<B"), aTabNames[3]);
    CPPUNIT_ASSERT_EQUAL(OUString("C>D"), aTabNames[4]);

    xDocSh->DoClose();
}


void ScFiltersTest::testTdf150599()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf150599.", FORMAT_DIF);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : #IND:?
    CPPUNIT_ASSERT_EQUAL(OUString("1"), rDoc.GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("32"), rDoc.GetString(ScAddress(31, 0, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testCommentSize()
{
    ScDocShellRef xDocSh = loadDoc(u"comment.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    ScAddress aPos(0,0,0);
    ScPostIt *pNote = rDoc.GetNote(aPos);
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

    rDoc.GetUndoManager()->Undo();

    CPPUNIT_ASSERT_EQUAL(rOldRect.getOpenWidth(), pCaption->GetLogicRect().getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(rOldRect.getOpenHeight(), pCaption->GetLogicRect().getOpenHeight());

    xDocSh->DoClose();
}

static void testEnhancedProtectionImpl( const ScDocument& rDoc )
{
    const ScTableProtection* pProt = rDoc.GetTabProtection(0);

    CPPUNIT_ASSERT( pProt);

    CPPUNIT_ASSERT( !pProt->isBlockEditable( ScRange( 0, 0, 0, 0, 0, 0)));  // locked
    CPPUNIT_ASSERT(  pProt->isBlockEditable( ScRange( 0, 1, 0, 0, 1, 0)));  // editable without password
    CPPUNIT_ASSERT(  pProt->isBlockEditable( ScRange( 0, 2, 0, 0, 2, 0)));  // editable without password
    CPPUNIT_ASSERT( !pProt->isBlockEditable( ScRange( 0, 3, 0, 0, 3, 0)));  // editable with password "foo"
    CPPUNIT_ASSERT( !pProt->isBlockEditable( ScRange( 0, 4, 0, 0, 4, 0)));  // editable with descriptor
    CPPUNIT_ASSERT( !pProt->isBlockEditable( ScRange( 0, 5, 0, 0, 5, 0)));  // editable with descriptor and password "foo"
    CPPUNIT_ASSERT(  pProt->isBlockEditable( ScRange( 0, 1, 0, 0, 2, 0)));  // union of two different editables
    CPPUNIT_ASSERT( !pProt->isBlockEditable( ScRange( 0, 0, 0, 0, 1, 0)));  // union of locked and editable
    CPPUNIT_ASSERT( !pProt->isBlockEditable( ScRange( 0, 2, 0, 0, 3, 0)));  // union of editable and password editable
}

void ScFiltersTest::testEnhancedProtectionXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"enhanced-protection.", FORMAT_XLS);
    ScDocument& rDoc = xDocSh->GetDocument();

    testEnhancedProtectionImpl( rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testEnhancedProtectionXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"enhanced-protection.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();

    testEnhancedProtectionImpl( rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testSortWithSharedFormulasODS()
{
    ScDocShellRef xDocSh = loadDoc(u"shared-formula/sort-crash.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // E2:E10 should be shared.
    const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(4,1,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), pFC->GetSharedLength());

    // E12:E17 should be shared.
    pFC = rDoc.GetFormulaCell(ScAddress(4,11,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(11), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedLength());

    // Set A1:E17 as an anonymous database range to sheet, or else Calc would
    // refuse to sort the range.
    std::unique_ptr<ScDBData> pDBData(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 4, 16, true, true));
    rDoc.SetAnonymousDBData(0, std::move(pDBData));

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
    ScDBDocFunc aFunc(*xDocSh);
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // After the sort, E2:E16 should be shared.
    pFC = rDoc.GetFormulaCell(ScAddress(4,1,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedLength());

    xDocSh->DoClose();
}

// https://bugs.freedesktop.org/attachment.cgi?id=100089 from fdo#77018
// mentioned also in fdo#79441
// Document contains cached external references.
void ScFiltersTest::testSortWithSheetExternalReferencesODS()
{
    ScDocShellRef xDocSh = loadDoc(u"sort-with-sheet-external-references.", FORMAT_ODS, true);
    ScDocument& rDoc = xDocSh->GetDocument();
    sc::AutoCalcSwitch aACSwitch(rDoc, true); // turn auto calc on.
    rDoc.CalcAll();

    // We reset the SortRefUpdate value back to the original in tearDown().
    ScInputOptions aInputOption = SC_MOD()->GetInputOptions();

    // The complete relative test only works with UpdateReferenceOnSort==true,
    // but the internal and external sheet references have to work in both
    // modes.

    aInputOption.SetSortRefUpdate(true);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A15:D20 with relative row references. UpdateReferenceOnSort==true
    // With in-sheet relative references.
    testSortWithSheetExternalReferencesODS_Impl( xDocSh, 14, 19, true);

    // Undo sort with relative references to perform same sort.
    rDoc.GetUndoManager()->Undo();
    rDoc.CalcAll();

    aInputOption.SetSortRefUpdate(false);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A15:D20 with relative row references. UpdateReferenceOnSort==false
    // Without in-sheet relative references.
    testSortWithSheetExternalReferencesODS_Impl( xDocSh, 14, 19, false);

    // Undo sort with relative references to perform new sort.
    rDoc.GetUndoManager()->Undo();
    rDoc.CalcAll();

    // Sort with absolute references has to work in both UpdateReferenceOnSort
    // modes.

    aInputOption.SetSortRefUpdate(true);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A23:D28 with absolute row references. UpdateReferenceOnSort==true
    // With in-sheet relative references.
    testSortWithSheetExternalReferencesODS_Impl( xDocSh, 22, 27, true);

    // Undo sort with absolute references to perform same sort.
    rDoc.GetUndoManager()->Undo();
    rDoc.CalcAll();

    aInputOption.SetSortRefUpdate(false);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A23:D28 with absolute row references. UpdateReferenceOnSort==false
    // With in-sheet relative references.
    testSortWithSheetExternalReferencesODS_Impl( xDocSh, 22, 27, true);

    xDocSh->DoClose();
}

void ScFiltersTest::testSortWithSheetExternalReferencesODS_Impl( ScDocShellRef const & xDocSh, SCROW nRow1, SCROW nRow2,
        bool bCheckRelativeInSheet )
{
    ScDocument& rDoc = xDocSh->GetDocument();

    // Check the original data is there.
    for (SCROW nRow=nRow1+1; nRow <= nRow2; ++nRow)
    {
        double const aCheck[] = { 1, 2, 3, 4, 5 };
        CPPUNIT_ASSERT_EQUAL( aCheck[nRow-nRow1-1], rDoc.GetValue( ScAddress(0,nRow,0)));
    }
    for (SCROW nRow=nRow1+1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol=1; nCol <= 3; ++nCol)
        {
            double const aCheck[] = { 1, 12, 123, 1234, 12345 };
            CPPUNIT_ASSERT_EQUAL( aCheck[nRow-nRow1-1], rDoc.GetValue( ScAddress(nCol,nRow,0)));
        }
    }

    // Set as an anonymous database range to sort.
    std::unique_ptr<ScDBData> pDBData(new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, nRow1, 3, nRow2, true, true));
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
    ScDBDocFunc aFunc(*xDocSh);
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);
    rDoc.CalcAll();

    // Check the sort and that all sheet references and external references are
    // adjusted to point to the original location.
    for (SCROW nRow=nRow1+1; nRow <= nRow2; ++nRow)
    {
        double const aCheck[] = { 5, 4, 3, 2, 1 };
        CPPUNIT_ASSERT_EQUAL( aCheck[nRow-nRow1-1], rDoc.GetValue( ScAddress(0,nRow,0)));
    }
    // The last column (D) are in-sheet relative references.
    SCCOL nEndCol = (bCheckRelativeInSheet ? 3 : 2);
    for (SCROW nRow=nRow1+1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol=1; nCol <= nEndCol; ++nCol)
        {
            double const aCheck[] = { 12345, 1234, 123, 12, 1 };
            CPPUNIT_ASSERT_EQUAL( aCheck[nRow-nRow1-1], rDoc.GetValue( ScAddress(nCol,nRow,0)));
        }
    }
}

void ScFiltersTest::testSortWithFormattingXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf129127.", FORMAT_XLS, true);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Set as an anonymous database range to sort.
    std::unique_ptr<ScDBData> pDBData(
        new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 4, 9, false, false));
    rDoc.SetAnonymousDBData(0, std::move(pDBData));

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
    ScDBDocFunc aFunc(*xDocSh);
    // Without the fix, sort would crash.
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);
    xDocSh->DoClose();
}

void ScFiltersTest::testTooManyColsRows()
{
    // The intentionally doc has cells beyond our MAXROW/MAXCOL, so there
    // should be a warning on load.
    ScDocShellRef xDocSh = loadDoc(u"too-many-cols-rows.", FORMAT_ODS, /*bReadWrite*/ false,
                                   /*bCheckErrorCode*/ false);
    CPPUNIT_ASSERT(xDocSh->GetErrorCode() == SCWARN_IMPORT_ROW_OVERFLOW
                   || xDocSh->GetErrorCode() == SCWARN_IMPORT_COLUMN_OVERFLOW);
    xDocSh->DoClose();

    xDocSh = loadDoc(u"too-many-cols-rows.", FORMAT_XLSX, /*bReadWrite*/ false,
                     /*bCheckErrorCode*/ false);
    CPPUNIT_ASSERT(xDocSh->GetErrorCode() == SCWARN_IMPORT_ROW_OVERFLOW
                   || xDocSh->GetErrorCode() == SCWARN_IMPORT_COLUMN_OVERFLOW);
    xDocSh->DoClose();
}

// just needs to not crash on recalc
void ScFiltersTest::testForcepoint107()
{
    ScDocShellRef xDocSh = loadDoc(u"forcepoint107.", FORMAT_XLSX, true);
    xDocSh->DoHardRecalc();
}

ScFiltersTest::ScFiltersTest()
    : ScBootstrapFixture( "sc/qa/unit/data" )
    , mbUpdateReferenceOnSort(false)
{
}

void ScFiltersTest::setUp()
{
    ScBootstrapFixture::setUp();

    // one test sets this configuration option; make sure we remember the
    // original value
    ScInputOptions aInputOption = SC_MOD()->GetInputOptions();
    mbUpdateReferenceOnSort = aInputOption.GetSortRefUpdate();
}

void ScFiltersTest::tearDown()
{
    // one test sets this configuration option; make sure we return it back
    ScInputOptions aInputOption = SC_MOD()->GetInputOptions();
    if (mbUpdateReferenceOnSort != aInputOption.GetSortRefUpdate())
    {
        aInputOption.SetSortRefUpdate(mbUpdateReferenceOnSort);
        SC_MOD()->SetInputOptions(aInputOption);
    }

    ScBootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
