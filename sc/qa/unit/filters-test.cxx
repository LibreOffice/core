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
#include <scopetools.hxx>
#include <scmod.hxx>

#include <svx/svdpage.hxx>

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
    void testRangeNameODS(); // only test ods here, xls and xlsx in subsequent_filters-test
    void testContentODS();
    void testContentXLS();
    void testContentXLSX();
    void testContentXLSXStrict(); // strict OOXML
    void testContentLotus123();
    void testContentofz9704();
    void testContentDIF();
    void testContentXLSB();
    void testContentXLS_XML();
    void testContentGnumeric();
    void testSharedFormulaXLS();
    void testSharedFormulaXLSX();
    void testSheetNamesXLSX();
    void testLegacyCellAnchoredRotatedShape();
    void testEnhancedProtectionXLS();
    void testEnhancedProtectionXLSX();
    void testSortWithSharedFormulasODS();
    void testSortWithSheetExternalReferencesODS();
    void testSortWithSheetExternalReferencesODS_Impl( ScDocShellRef const & xDocShRef, SCROW nRow1, SCROW nRow2,
            bool bCheckRelativeInSheet );

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testRangeNameODS);
    CPPUNIT_TEST(testContentODS);
    CPPUNIT_TEST(testContentXLS);
    CPPUNIT_TEST(testContentXLSX);
    CPPUNIT_TEST(testContentXLSXStrict);
    CPPUNIT_TEST(testContentLotus123);
    CPPUNIT_TEST(testContentofz9704);
    CPPUNIT_TEST(testContentDIF);
    CPPUNIT_TEST(testContentXLSB);
    CPPUNIT_TEST(testContentXLS_XML);
    CPPUNIT_TEST(testContentGnumeric);
    CPPUNIT_TEST(testSharedFormulaXLS);
    CPPUNIT_TEST(testSharedFormulaXLSX);
    CPPUNIT_TEST(testSheetNamesXLSX);
    CPPUNIT_TEST(testLegacyCellAnchoredRotatedShape);
    CPPUNIT_TEST(testEnhancedProtectionXLS);
    CPPUNIT_TEST(testEnhancedProtectionXLSX);
    CPPUNIT_TEST(testSortWithSharedFormulasODS);
    CPPUNIT_TEST(testSortWithSheetExternalReferencesODS);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
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
        m_directories.getURLFromSrc("/sc/qa/unit/data/qpro/"));

    //warning, the current "sylk filter" in sc (docsh.cxx) automatically
    //chains on failure on trying as csv, rtf, etc. so "success" may
    //not indicate that it imported as .slk.
    testDir("SYLK",
        m_directories.getURLFromSrc("/sc/qa/unit/data/slk/"));

    testDir("MS Excel 97",
        m_directories.getURLFromSrc("/sc/qa/unit/data/xls/"));

    testDir("Calc Office Open XML",
        m_directories.getURLFromSrc("/sc/qa/unit/data/xlsx/"), OUString(), XLSX_FORMAT_TYPE);

    testDir("Calc Office Open XML",
        m_directories.getURLFromSrc("/sc/qa/unit/data/xlsm/"), OUString(), XLSX_FORMAT_TYPE);

    testDir("dBase",
        m_directories.getURLFromSrc("/sc/qa/unit/data/dbf/"));

    testDir("Lotus",
        m_directories.getURLFromSrc("/sc/qa/unit/data/wks/"));

#endif
}

namespace {

void testRangeNameImpl(const ScDocument& rDoc)
{
    //check one range data per sheet and one global more detailed
    //add some more checks here
    ScRangeData* pRangeData = rDoc.GetRangeName()->findByUpperName(OUString("GLOBAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Global1 not found", pRangeData);
    double aValue;
    rDoc.GetValue(1,0,0,aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("range name Global1 should reference Sheet1.A1", 1.0, aValue);
    pRangeData = rDoc.GetRangeName(0)->findByUpperName(OUString("LOCAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 not found", pRangeData);
    rDoc.GetValue(1,2,0,aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("range name Sheet1.Local1 should reference Sheet1.A3", 3.0, aValue);
    pRangeData = rDoc.GetRangeName(1)->findByUpperName(OUString("LOCAL2"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet2.Local2 not found", pRangeData);
    //check for correct results for the remaining formulas
    rDoc.GetValue(1,1,0, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=global2 should be 2", 2.0, aValue);
    rDoc.GetValue(1,3,0, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=local2 should be 4", 4.0, aValue);
    rDoc.GetValue(2,0,0, aValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=SUM(global3) should be 10", 10.0, aValue);
}

}

void ScFiltersTest::testRangeNameODS()
{
    ScDocShellRef xDocSh = loadDoc("named-ranges-global.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load named-ranges-global.*", xDocSh.is());

    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    testRangeNameImpl(rDoc);

    OUString aCSVPath;
    createCSVPath( "rangeExp_Sheet2.", aCSVPath );
    testFile( aCSVPath, rDoc, 1);
    xDocSh->DoClose();
}

namespace {

void testContentImpl(ScDocument& rDoc, sal_Int32 nFormat ) //same code for ods, xls, xlsx
{
    double fValue;
    //check value import
    rDoc.GetValue(0,0,0,fValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("value not imported correctly", 1.0, fValue);
    rDoc.GetValue(0,1,0,fValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("value not imported correctly", 2.0, fValue);
    OUString aString = rDoc.GetString(1, 0, 0);

    //check string import
    CPPUNIT_ASSERT_EQUAL_MESSAGE("string imported not correctly", OUString("String1"), aString);
    aString = rDoc.GetString(1, 1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("string not imported correctly", OUString("String2"), aString);

    //check basic formula import
    // in case of DIF it just contains values
    rDoc.GetValue(2,0,0,fValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=2*3", 6.0, fValue);
    rDoc.GetValue(2,1,0,fValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=2+3", 5.0, fValue);
    rDoc.GetValue(2,2,0,fValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=2-3", -1.0, fValue);
    rDoc.GetValue(2,3,0,fValue);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=C1+C2", 11.0, fValue);

    //check merged cells import
    if (nFormat != FORMAT_LOTUS123 && nFormat != FORMAT_DIF && nFormat != FORMAT_XLS_XML
            && nFormat != FORMAT_GNUMERIC)
    {
        SCCOL nCol = 4;
        SCROW nRow = 1;
        rDoc.ExtendMerge(4, 1, nCol, nRow, 0);
        CPPUNIT_ASSERT_MESSAGE("merged cells are not imported", nCol == 5 && nRow == 2);

        //check notes import
        ScAddress aAddress(7, 2, 0);
        ScPostIt* pNote = rDoc.GetNote(aAddress);
        CPPUNIT_ASSERT_MESSAGE("note not imported", pNote);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("note text not imported correctly", OUString("Test"), pNote->GetText() );
    }

    //add additional checks here
}

}

void ScFiltersTest::testContentODS()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", FORMAT_ODS);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    testContentImpl(rDoc, FORMAT_ODS);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentXLS()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", FORMAT_XLS);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    testContentImpl(rDoc, FORMAT_XLS);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentXLSX()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", FORMAT_XLSX);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    testContentImpl(rDoc, FORMAT_XLSX);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentXLSXStrict()
{
    ScDocShellRef xDocSh = loadDoc("universal-content-strict.", FORMAT_XLSX);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    testContentImpl(rDoc, FORMAT_XLSX);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentLotus123()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", FORMAT_LOTUS123);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    testContentImpl(rDoc, FORMAT_LOTUS123);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentofz9704()
{
    OUString aFileName;
    createFileURL("ofz9704.", "123", aFileName);
    SvFileStream aFileStream(aFileName, StreamMode::READ);
    TestImportWKS(aFileStream);
}

void ScFiltersTest::testContentDIF()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", FORMAT_DIF);

    CPPUNIT_ASSERT_MESSAGE("Failed to load universal-content.dif", xDocSh.is());

    xDocSh->DoClose();
}

void ScFiltersTest::testContentXLSB()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", FORMAT_XLSB);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    testContentImpl(rDoc, FORMAT_XLSB);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentXLS_XML()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", FORMAT_XLS_XML);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    testContentImpl(rDoc, FORMAT_XLS_XML);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentGnumeric()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", FORMAT_GNUMERIC);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    testContentImpl(rDoc, FORMAT_GNUMERIC);
    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaXLS()
{
    ScDocShellRef xDocSh = loadDoc("shared-formula/basic.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());
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
    CPPUNIT_ASSERT_MESSAGE("Incorrect group geometry.", xGroup->mpTopCell->aPos.Row() == 1 && xGroup->mnLength == 18);

    xDocSh->DoClose();

    // The following file contains shared formula whose range is inaccurate.
    // Excel can easily mess up shared formula ranges, so we need to be able
    // to handle these wrong ranges that Excel stores.

    xDocSh = loadDoc("shared-formula/gap.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());
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
    ScDocShellRef xDocSh = loadDoc("shared-formula/basic.", FORMAT_XLSX);
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
    CPPUNIT_ASSERT_MESSAGE("Incorrect group geometry.", xGroup->mpTopCell->aPos.Row() == 1 && xGroup->mnLength == 18);

    xDocSh->DoClose();
}

void ScFiltersTest::testSheetNamesXLSX()
{
    ScDocShellRef xDocSh = loadDoc("sheet-names.", FORMAT_XLSX);
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

static void impl_testLegacyCellAnchoredRotatedShape( ScDocument& rDoc, const tools::Rectangle& aRect, const ScDrawObjData& aAnchor, long TOLERANCE = 30 /* 30 hmm */ )
{
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No drawing layer.", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 1st sheet.", pPage);
    CPPUNIT_ASSERT_EQUAL( static_cast<size_t>(1), pPage->GetObjCount() );

    SdrObject* pObj = pPage->GetObj(0);
    const tools::Rectangle& aSnap = pObj->GetSnapRect();
    printf("expected height %ld actual %ld\n", aRect.GetHeight(), aSnap.GetHeight() );
    CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRect.GetHeight(), aSnap.GetHeight(), TOLERANCE ) );
    printf("expected width %ld actual %ld\n", aRect.GetWidth(), aSnap.GetWidth() );
    CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRect.GetWidth(), aSnap.GetWidth(), TOLERANCE ) );
    printf("expected left %ld actual %ld\n", aRect.Left(), aSnap.Left() );
    CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRect.Left(), aSnap.Left(), TOLERANCE ) );
    printf("expected right %ld actual %ld\n", aRect.Top(), aSnap.Top() );
    CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRect.Top(), aSnap.Top(), TOLERANCE ) );

    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
    CPPUNIT_ASSERT_MESSAGE("expected object meta data", pData);
    printf("expected startrow %" SAL_PRIdINT32 " actual %" SAL_PRIdINT32 "\n", aAnchor.maStart.Row(), pData->maStart.Row()  );
    CPPUNIT_ASSERT_EQUAL( aAnchor.maStart.Row(), pData->maStart.Row() );
    printf("expected startcol %d actual %d\n", aAnchor.maStart.Col(), pData->maStart.Col()  );
    CPPUNIT_ASSERT_EQUAL( aAnchor.maStart.Col(), pData->maStart.Col() );
    printf("expected endrow %" SAL_PRIdINT32 " actual %" SAL_PRIdINT32 "\n", aAnchor.maEnd.Row(), pData->maEnd.Row()  );
    CPPUNIT_ASSERT_EQUAL( aAnchor.maEnd.Row(), pData->maEnd.Row() );
    printf("expected endcol %d actual %d\n", aAnchor.maEnd.Col(), pData->maEnd.Col()  );
    CPPUNIT_ASSERT_EQUAL( aAnchor.maEnd.Col(), pData->maEnd.Col() );
}

void ScFiltersTest::testLegacyCellAnchoredRotatedShape()
{
    {
        // This example doc contains cell anchored shape that is rotated, the
        // rotated shape is in fact clipped by the sheet boundaries (and thus
        // is a good edge case test to see if we import it still correctly)
        ScDocShellRef xDocSh = loadDoc("legacycellanchoredrotatedclippedshape.", FORMAT_ODS);

        ScDocument& rDoc = xDocSh->GetDocument();
        // ensure the imported legacy rotated shape is in the expected position
        tools::Rectangle aRect( 6000, -2000, 8000, 4000 );
        // ensure the imported ( and converted ) anchor ( note we internally now store the anchor in
        // terms of the rotated shape ) is more or less contains the correct info
        ScDrawObjData aAnchor;
        aAnchor.maStart.SetRow( 0 );
        aAnchor.maStart.SetCol( 5 );
        aAnchor.maEnd.SetRow( 3 );
        aAnchor.maEnd.SetCol( 7 );
        impl_testLegacyCellAnchoredRotatedShape( rDoc, aRect, aAnchor );
        // test save and reload
        // for some reason having this test in subsequent_export-test.cxx causes
        // a core dump in editeng ( so moved to here )
        xDocSh = saveAndReload( &(*xDocSh), FORMAT_ODS);
        ScDocument& rDoc2 = xDocSh->GetDocument();
        impl_testLegacyCellAnchoredRotatedShape( rDoc2, aRect, aAnchor );

        xDocSh->DoClose();
    }
    {
        // This example doc contains cell anchored shape that is rotated, the
        // rotated shape is in fact clipped by the sheet boundaries, additionally
        // the shape is completely hidden because the rows the shape occupies
        // are hidden
        ScDocShellRef xDocSh = loadDoc("legacycellanchoredrotatedhiddenshape.", FORMAT_ODS, true);
        ScDocument& rDoc = xDocSh->GetDocument();
        // ensure the imported legacy rotated shape is in the expected position
        // when a shape is fully hidden reloading seems to result is in some errors, usually
        // ( same but different error happens pre-patch ) - we should do better here, I regard it
        // as a pre-existing bug though (#FIXME)
        //Rectangle aRect( 6000, -2000, 8000, 4000 ); // proper dimensions
        tools::Rectangle aRect( 6000, -2000, 7430, 4000 );
        // ensure the imported (and converted) anchor (note we internally now store the anchor in
        // terms of the rotated shape) is more or less contains the correct info
        ScDrawObjData aAnchor;
        aAnchor.maStart.SetRow( 0 );
        aAnchor.maStart.SetCol( 5 );
        aAnchor.maEnd.SetRow( 3 );
        aAnchor.maEnd.SetCol( 7 );
        rDoc.ShowRows(0, 9, 0, true); // show relevant rows
        rDoc.SetDrawPageSize(0); // trigger recalcpos

        // apply hefty (1 mm) tolerance here, as some opensuse tinderbox
        // failing
        impl_testLegacyCellAnchoredRotatedShape( rDoc, aRect, aAnchor, 100 );

        xDocSh->DoClose();
    }
    {
        // This example doc contains cell anchored shape that is rotated
        ScDocShellRef xDocSh = loadDoc("legacycellanchoredrotatedshape.", FORMAT_ODS);

        ScDocument& rDoc = xDocSh->GetDocument();
        // ensure the imported legacy rotated shape is in the expected position
        tools::Rectangle aRect( 6000, 3000, 8000, 9000 );
        // ensure the imported (and converted) anchor (note we internally now store the anchor in
        // terms of the rotated shape) more or less contains the correct info

        ScDrawObjData aAnchor;
        aAnchor.maStart.SetRow( 3 );
        aAnchor.maStart.SetCol( 6 );
        aAnchor.maEnd.SetRow( 9 );
        aAnchor.maEnd.SetCol( 7 );
        // test import
        impl_testLegacyCellAnchoredRotatedShape( rDoc, aRect, aAnchor );
        // test save and reload
        xDocSh = saveAndReload( &(*xDocSh), FORMAT_ODS);
        ScDocument& rDoc2 = xDocSh->GetDocument();
        impl_testLegacyCellAnchoredRotatedShape( rDoc2, aRect, aAnchor );

        xDocSh->DoClose();
    }
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
    ScDocShellRef xDocSh = loadDoc("enhanced-protection.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    testEnhancedProtectionImpl( rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testEnhancedProtectionXLSX()
{
    ScDocShellRef xDocSh = loadDoc("enhanced-protection.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    testEnhancedProtectionImpl( rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testSortWithSharedFormulasODS()
{
    ScDocShellRef xDocSh = loadDoc("shared-formula/sort-crash.", FORMAT_ODS, true);
    CPPUNIT_ASSERT(xDocSh.is());
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
    ScDocShellRef xDocSh = loadDoc("sort-with-sheet-external-references.", FORMAT_ODS, true);
    CPPUNIT_ASSERT(xDocSh.is());
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

ScFiltersTest::ScFiltersTest()
    : ScBootstrapFixture( "sc/qa/unit/data" )
    , mbUpdateReferenceOnSort(false)
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

    // one test sets this configuration option; make sure we remember the
    // original value
    ScInputOptions aInputOption = SC_MOD()->GetInputOptions();
    mbUpdateReferenceOnSort = aInputOption.GetSortRefUpdate();
}

void ScFiltersTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();

    // one test sets this configuration option; make sure we return it back
    ScInputOptions aInputOption = SC_MOD()->GetInputOptions();
    if (mbUpdateReferenceOnSort != aInputOption.GetSortRefUpdate())
    {
        aInputOption.SetSortRefUpdate(mbUpdateReferenceOnSort);
        SC_MOD()->SetInputOptions(aInputOption);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
