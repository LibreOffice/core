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

#include "scdll.hxx"
#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>

#define TEST_BUG_FILES 0

#include "helper/qahelper.hxx"

#include "docsh.hxx"
#include "postit.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"
#include "cellform.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
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

    virtual void setUp();
    virtual void tearDown();

    virtual bool load( const OUString &rFilter, const OUString &rURL,
        const OUString &rUserData, unsigned int nFilterFlags,
        unsigned int nClipboardID, unsigned int nFilterVersion);
    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    //ods, xls, xlsx filter tests
    void testRangeNameODS(); // only test ods here, xls and xlsx in subsequent_filters-test
    void testContentODS();
    void testContentXLS();
    void testContentXLSX();
    void testContentLotus123();
    void testContentDIF();
    void testSharedFormulaXLS();
    void testSharedFormulaXLSX();
#if TEST_BUG_FILES
    //goes recursively through all files in this dir and tries to open them
    void testDir(osl::Directory& rDir, sal_Int32 nType);
    //test Bug Files and search for files that crash LibO
    void testBugFiles();
    void testBugFilesXLS();
    void testBugFilesXLSX();
#endif
    void testLegacyCellAnchoredRotatedShape();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testRangeNameODS);
    CPPUNIT_TEST(testContentODS);
    CPPUNIT_TEST(testContentXLS);
    CPPUNIT_TEST(testContentXLSX);
    CPPUNIT_TEST(testContentLotus123);
    CPPUNIT_TEST(testContentDIF);
//  CPPUNIT_TEST(testSharedFormulaXLS);
//  CPPUNIT_TEST(testSharedFormulaXLSX);
    CPPUNIT_TEST(testLegacyCellAnchoredRotatedShape);

#if TEST_BUG_FILES
    CPPUNIT_TEST(testBugFiles);
    CPPUNIT_TEST(testBugFilesXLS);
    CPPUNIT_TEST(testBugFilesXLSX);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

bool ScFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, unsigned int nFilterFlags,
        unsigned int nClipboardID, unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load(rURL, rFilter, rUserData,
        OUString(), nFilterFlags, nClipboardID, nFilterVersion );
    bool bLoaded = xDocShRef.Is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

void ScFiltersTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir(OUString("Quattro Pro 6.0"),
        getURLFromSrc("/sc/qa/unit/data/qpro/"), OUString());

    //warning, the current "sylk filter" in sc (docsh.cxx) automatically
    //chains on failure on trying as csv, rtf, etc. so "success" may
    //not indicate that it imported as .slk.
    testDir(OUString("SYLK"),
        getURLFromSrc("/sc/qa/unit/data/slk/"), OUString());

    testDir(OUString("MS Excel 97"),
        getURLFromSrc("/sc/qa/unit/data/xls/"), OUString());
#endif
}

#if TEST_BUG_FILES

void ScFiltersTest::testDir(osl::Directory& rDir, sal_uInt32 nType)
{
    OUString aFilterName(getFileFormats()[nType].pFilterName, strlen(getFileFormats()[nType].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFilterType(getFileFormats()[nType].pTypeName, strlen(getFileFormats()[nType].pTypeName), RTL_TEXTENCODING_UTF8);

    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL|osl_FileStatus_Mask_Type);
    while (rDir.getNextItem(aItem) == osl::FileBase::E_None)
    {
        aItem.getFileStatus(aFileStatus);
        OUString sURL = aFileStatus.getFileURL();
        std::cout << "File: " << OUStringToOString(sURL, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
        //OStringBuffer aMessage("Failed loading: ");
        //aMessage.append(OUStringToOString(sURL, RTL_TEXTENCODING_UTF8));

        unsigned int nFormatType = getFileFormats()[nType].nFormatType;
        unsigned int nClipboardId = nFormatType ? SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS : 0;
        ScDocShellRef xDocSh = load(sURL, aFilterName, OUString(),
            aFilterType, nFormatType, nClipboardId );
        // use this only if you're sure that all files can be loaded
        // pay attention to lock files
        //CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), xDocSh.Is());
        if (xDocSh.Is())
            xDocSh->DoClose();
    }
}

void ScFiltersTest::testBugFiles()
{
    OUString aDirName = getURLFromSrc("/sc/qa/unit/data/bugODS/");
    osl::Directory aDir(aDirName);

    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.open());
    testDir(aDir, 0);
}

void ScFiltersTest::testBugFilesXLS()
{
    OUString aDirName = getURLFromSrc("/sc/qa/unit/data/bugXLS/");
    osl::Directory aDir(aDirName);

    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.open());
    testDir(aDir, 1);
}

void ScFiltersTest::testBugFilesXLSX()
{
    OUString aDirName = getURLFromSrc("/sc/qa/unit/data/bugXLSX/");
    osl::Directory aDir(aDirName);

    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.open());
    testDir(aDir, 2);
}

#endif

namespace {

void testRangeNameImpl(ScDocument* pDoc)
{
    //check one range data per sheet and one global more detailed
    //add some more checks here
    ScRangeData* pRangeData = pDoc->GetRangeName()->findByUpperName(OUString("GLOBAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Global1 not found", pRangeData);
    double aValue;
    pDoc->GetValue(1,0,0,aValue);
    CPPUNIT_ASSERT_MESSAGE("range name Global1 should reference Sheet1.A1", aValue == 1);
    pRangeData = pDoc->GetRangeName(0)->findByUpperName(OUString("LOCAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 not found", pRangeData);
    pDoc->GetValue(1,2,0,aValue);
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 should reference Sheet1.A3", aValue == 3);
    pRangeData = pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL2"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet2.Local2 not found", pRangeData);
    //check for correct results for the remaining formulas
    pDoc->GetValue(1,1,0, aValue);
    CPPUNIT_ASSERT_MESSAGE("=global2 should be 2", aValue == 2);
    pDoc->GetValue(1,3,0, aValue);
    CPPUNIT_ASSERT_MESSAGE("=local2 should be 4", aValue == 4);
    pDoc->GetValue(2,0,0, aValue);
    CPPUNIT_ASSERT_MESSAGE("=SUM(global3) should be 10", aValue == 10);
}

}

void ScFiltersTest::testRangeNameODS()
{
    ScDocShellRef xDocSh = loadDoc("named-ranges-global.", ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load named-ranges-globals.*", xDocSh.Is());

    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testRangeNameImpl(pDoc);

    OUString aSheet2CSV("rangeExp_Sheet2.");
    OUString aCSVPath;
    createCSVPath( aSheet2CSV, aCSVPath );
    testFile( aCSVPath, pDoc, 1);
    xDocSh->DoClose();
}

namespace {

void testContentImpl(ScDocument* pDoc, sal_Int32 nFormat ) //same code for ods, xls, xlsx
{
    double fValue;
    //check value import
    pDoc->GetValue(0,0,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("value not imported correctly", fValue == 1);
    pDoc->GetValue(0,1,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("value not imported correctly", fValue == 2);
    OUString aString = pDoc->GetString(1, 0, 0);

    //check string import
    CPPUNIT_ASSERT_MESSAGE("string imported not correctly", aString == OUString("String1"));
    aString = pDoc->GetString(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("string not imported correctly", aString == OUString("String2"));

    //check basic formula import
    // in case of DIF it just contains values
    pDoc->GetValue(2,0,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("=2*3", fValue == 6);
    pDoc->GetValue(2,1,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("=2+3", fValue == 5);
    pDoc->GetValue(2,2,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("=2-3", fValue == -1);
    pDoc->GetValue(2,3,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("=C1+C2", fValue == 11);

    //check merged cells import
    if(nFormat != LOTUS123 && nFormat != DIF)
    {
        SCCOL nCol = 4;
        SCROW nRow = 1;
        pDoc->ExtendMerge(4, 1, nCol, nRow, 0, false);
        CPPUNIT_ASSERT_MESSAGE("merged cells are not imported", nCol == 5 && nRow == 2);

        //check notes import
        ScAddress aAddress(7, 2, 0);
        ScPostIt* pNote = pDoc->GetNotes(aAddress.Tab())->findByAddress(aAddress);
        CPPUNIT_ASSERT_MESSAGE("note not imported", pNote);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("note text not imported correctly", pNote->GetText(), OUString("Test"));
    }

    //add additional checks here
}

}

void ScFiltersTest::testContentODS()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", ODS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testContentImpl(pDoc, ODS);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentXLS()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", XLS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testContentImpl(pDoc, XLS);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentXLSX()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", XLSX);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testContentImpl(pDoc, XLSX);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentLotus123()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", LOTUS123);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    testContentImpl(pDoc, LOTUS123);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentDIF()
{
    ScDocShellRef xDocSh = loadDoc("universal-content.", DIF);

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaXLS()
{
    ScDocShellRef xDocSh = loadDoc("shared-formula.", XLS);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 18; ++i)
    {
        double fVal = pDoc->GetValue(ScAddress(1,i,0));
        double fCheck = i*10.0;
        CPPUNIT_ASSERT_EQUAL(fCheck, fVal);
    }
    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaXLSX()
{
    ScDocShellRef xDocSh = loadDoc("shared-formula.", XLSX);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoHardRecalc(true);
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 18; ++i)
    {
        double fVal = pDoc->GetValue(ScAddress(1,i,0));
        double fCheck = i*10.0;
        CPPUNIT_ASSERT_EQUAL(fCheck, fVal);
    }
    xDocSh->DoClose();
}

void impl_testLegacyCellAnchoredRotatedShape( ScDocument* pDoc, Rectangle& aRect, ScDrawObjData& aAnchor, long TOLERANCE = 30 /* 30 hmm */ )
{
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No drawing layer.", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 1st sheet.", pPage);
    CPPUNIT_ASSERT_EQUAL( sal_uIntPtr(1), pPage->GetObjCount() );

    SdrObject* pObj = pPage->GetObj(0);
    const Rectangle& aSnap = pObj->GetSnapRect();
    printf("expected height %ld actual %ld\n", aRect.GetHeight(), aSnap.GetHeight() );
    CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRect.GetHeight(), aSnap.GetHeight(), TOLERANCE ) );
    printf("expected width %ld actual %ld\n", aRect.GetWidth(), aSnap.GetWidth() );
    CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRect.GetWidth(), aSnap.GetWidth(), TOLERANCE ) );
    printf("expected left %ld actual %ld\n", aRect.Left(), aSnap.Left() );
    CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRect.Left(), aSnap.Left(), TOLERANCE ) );
    printf("expected right %ld actual %ld\n", aRect.Top(), aSnap.Top() );
    CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aRect.Top(), aSnap.Top(), TOLERANCE ) );


    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
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
        // rotated shape is in fact cliped by the sheet boundries ( and thus
        // is a good edge case test to see if we import it still correctly )
        ScDocShellRef xDocSh = loadDoc("legacycellanchoredrotatedclippedshape.", ODS);

        ScDocument* pDoc = xDocSh->GetDocument();
        CPPUNIT_ASSERT(pDoc);
        // ensure the imported legacy rotated shape is in the expected position
        Rectangle aRect( 6000, -2000, 8000, 4000 );
        // ensure the imported ( and converted ) anchor ( note we internally now store the anchor in
        // terms of the rotated shape ) is more or less contains the correct info
        ScDrawObjData aAnchor;
        aAnchor.maStart.SetRow( 0 );
        aAnchor.maStart.SetCol( 5 );
        aAnchor.maEnd.SetRow( 3 );
        aAnchor.maEnd.SetCol( 7 );
        impl_testLegacyCellAnchoredRotatedShape( pDoc, aRect, aAnchor );
        // test save and reload
        // for some reason having this test in subsequent_export-test.cxx causes
        // a core dump in editeng ( so moved to here )
        xDocSh = saveAndReload( &(*xDocSh), ODS);
        pDoc = xDocSh->GetDocument();
        CPPUNIT_ASSERT(pDoc);
        impl_testLegacyCellAnchoredRotatedShape( pDoc, aRect, aAnchor );
    }
    {
        // This example doc contains cell anchored shape that is rotated, the
        // rotated shape is in fact clipped by the sheet boundries, additionally
        // the shape is completely hidden because the rows the shape occupies
        // are hidden
        ScDocShellRef xDocSh = loadDoc("legacycellanchoredrotatedhiddenshape.", ODS, true);
        ScDocument* pDoc = xDocSh->GetDocument();
        CPPUNIT_ASSERT(pDoc);
        // ensure the imported legacy rotated shape is in the expected position
        // when a shape is fully hidden reloading seems to result is in some errors, usually
        // ( same but different error happens pre-patch ) - we should do better here, I regard it
        // as a pre-existing bug though ( #FIXME )
        //Rectangle aRect( 6000, -2000, 8000, 4000 ); // proper dimensions
        Rectangle aRect( 6000, -2000, 7430, 4000 );
        // ensure the imported ( and converted ) anchor ( note we internally now store the anchor in
        // terms of the rotated shape ) is more or less contains the correct info
        ScDrawObjData aAnchor;
        aAnchor.maStart.SetRow( 0 );
        aAnchor.maStart.SetCol( 5 );
        aAnchor.maEnd.SetRow( 3 );
        aAnchor.maEnd.SetCol( 7 );
        pDoc->ShowRows(0, 9, 0, true); // show relavent rows
        pDoc->SetDrawPageSize(0); // trigger recalcpos

        // apply hefty ( 1 mm ) tolerence here, as some opensuse tinderbox
        // failing
        impl_testLegacyCellAnchoredRotatedShape( pDoc, aRect, aAnchor, 100 );
        xDocSh->DoClose();
    }
    {
        // This example doc contains cell anchored shape that is rotated
        ScDocShellRef xDocSh = loadDoc("legacycellanchoredrotatedshape.", ODS);

        ScDocument* pDoc = xDocSh->GetDocument();
        CPPUNIT_ASSERT(pDoc);
        // ensure the imported legacy rotated shape is in the expected position
        Rectangle aRect( 6000, 3000, 8000, 9000 );
        // ensure the imported ( and converted ) anchor ( note we internally now store the anchor in
        // terms of the rotated shape ) is more or less contains the correct info

        ScDrawObjData aAnchor;
        aAnchor.maStart.SetRow( 3 );
        aAnchor.maStart.SetCol( 6 );
        aAnchor.maEnd.SetRow( 9 );
        aAnchor.maEnd.SetCol( 7 );
        // test import
        impl_testLegacyCellAnchoredRotatedShape( pDoc, aRect, aAnchor );
        // test save and reload
        xDocSh = saveAndReload( &(*xDocSh), ODS);
        pDoc = xDocSh->GetDocument();
        CPPUNIT_ASSERT(pDoc);
        impl_testLegacyCellAnchoredRotatedShape( pDoc, aRect, aAnchor );
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
