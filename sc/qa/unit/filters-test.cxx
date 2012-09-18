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
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com>
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

#define CALC_DEBUG_OUTPUT 0
#define TEST_BUG_FILES 0

#include "helper/qahelper.hxx"

#include "docsh.hxx"
#include "postit.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"
#include "cellform.hxx"

#define ODS_FORMAT_TYPE 50331943
#define XLS_FORMAT_TYPE 318767171
#define XLSX_FORMAT_TYPE 268959811
#define LOTUS123_FORMAT_TYPE 268435649

#define ODS     0
#define XLS     1
#define XLSX    2
#define LOTUS123 3

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
    { "123" , "Lotus", "calc_Lotus", LOTUS123_FORMAT_TYPE }
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

#if TEST_BUG_FILES
    //goes recursively through all files in this dir and tries to open them
    void testDir(osl::Directory& rDir, sal_Int32 nType);
    //test Bug Files and search for files that crash LibO
    void testBugFiles();
    void testBugFilesXLS();
    void testBugFilesXLSX();
#endif

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testRangeNameODS);
    CPPUNIT_TEST(testContentODS);
    CPPUNIT_TEST(testContentXLS);
    CPPUNIT_TEST(testContentXLSX);
    CPPUNIT_TEST(testContentLotus123);

#if TEST_BUG_FILES
    CPPUNIT_TEST(testBugFiles);
    CPPUNIT_TEST(testBugFilesXLS);
    CPPUNIT_TEST(testBugFilesXLSX);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    ScDocShellRef loadDoc(const rtl::OUString& rName, sal_Int32 nFormat);
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

void ScFiltersTest::testCVEs()
{
    testDir(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Quattro Pro 6.0")),
        getURLFromSrc("/sc/qa/unit/data/qpro/"), rtl::OUString());

    //warning, the current "sylk filter" in sc (docsh.cxx) automatically
    //chains on failure on trying as csv, rtf, etc. so "success" may
    //not indicate that it imported as .slk.
    testDir(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYLK")),
        getURLFromSrc("/sc/qa/unit/data/slk/"), rtl::OUString());

    testDir(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MS Excel 97")),
        getURLFromSrc("/sc/qa/unit/data/xls/"), rtl::OUString());
}

#if TEST_BUG_FILES

void ScFiltersTest::testDir(osl::Directory& rDir, sal_uInt32 nType)
{
    rtl::OUString aFilterName(aFileFormats[nType].pFilterName, strlen(aFileFormats[nType].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFilterType(aFileFormats[nType].pTypeName, strlen(aFileFormats[nType].pTypeName), RTL_TEXTENCODING_UTF8);

    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL|osl_FileStatus_Mask_Type);
    while (rDir.getNextItem(aItem) == osl::FileBase::E_None)
    {
        aItem.getFileStatus(aFileStatus);
        rtl::OUString sURL = aFileStatus.getFileURL();
        std::cout << "File: " << rtl::OUStringToOString(sURL, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
        //rtl::OStringBuffer aMessage("Failed loading: ");
        //aMessage.append(rtl::OUStringToOString(sURL, RTL_TEXTENCODING_UTF8));
        ScDocShellRef xDocSh = load( aFilterName,sURL, rtl::OUString(),aFilterType, aFileFormats[nType].nFormatType);
        // use this only if you're sure that all files can be loaded
        // pay attention to lock files
        //CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), xDocSh.Is());
        if (xDocSh.Is())
            xDocSh->DoClose();
    }
}

void ScFiltersTest::testBugFiles()
{
    rtl::OUString aDirName = getURLFromSrc("/sc/qa/unit/data/bugODS/");
    osl::Directory aDir(aDirName);

    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.open());
    testDir(aDir, 0);
}

void ScFiltersTest::testBugFilesXLS()
{
    rtl::OUString aDirName = getURLFromSrc("/sc/qa/unit/data/bugXLS/");
    osl::Directory aDir(aDirName);

    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.open());
    testDir(aDir, 1);
}

void ScFiltersTest::testBugFilesXLSX()
{
    rtl::OUString aDirName = getURLFromSrc("/sc/qa/unit/data/bugXLSX/");
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
    ScRangeData* pRangeData = pDoc->GetRangeName()->findByUpperName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GLOBAL1")));
    CPPUNIT_ASSERT_MESSAGE("range name Global1 not found", pRangeData);
    double aValue;
    pDoc->GetValue(1,0,0,aValue);
    CPPUNIT_ASSERT_MESSAGE("range name Global1 should reference Sheet1.A1", aValue == 1);
    pRangeData = pDoc->GetRangeName(0)->findByUpperName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LOCAL1")));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 not found", pRangeData);
    pDoc->GetValue(1,2,0,aValue);
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 should reference Sheet1.A3", aValue == 3);
    pRangeData = pDoc->GetRangeName(1)->findByUpperName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LOCAL2")));
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

void ScFiltersTest::testRangeNameODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("named-ranges-global."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 0);

    CPPUNIT_ASSERT_MESSAGE("Failed to load named-ranges-globals.*", xDocSh.Is());

    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testRangeNameImpl(pDoc);

    rtl::OUString aSheet2CSV(RTL_CONSTASCII_USTRINGPARAM("rangeExp_Sheet2."));
    rtl::OUString aCSVPath;
    createCSVPath( aSheet2CSV, aCSVPath );
    testFile( aCSVPath, pDoc, 1);
    xDocSh->DoClose();
}

namespace {

void testContentImpl(ScDocument* pDoc ) //same code for ods, xls, xlsx
{
    double fValue;
    //check value import
    pDoc->GetValue(0,0,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("value not imported correctly", fValue == 1);
    pDoc->GetValue(0,1,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("value not imported correctly", fValue == 2);
    rtl::OUString aString;
    pDoc->GetString(1,0,0,aString);
    //check string import
    CPPUNIT_ASSERT_MESSAGE("string imported not correctly", aString == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("String1")));
    pDoc->GetString(1,1,0,aString);
    CPPUNIT_ASSERT_MESSAGE("string not imported correctly", aString == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("String2")));
    //check basic formula import
    pDoc->GetValue(2,0,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("=2*3", fValue == 6);
    pDoc->GetValue(2,1,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("=2+3", fValue == 5);
    pDoc->GetValue(2,2,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("=2-3", fValue == -1);
    pDoc->GetValue(2,3,0,fValue);
    CPPUNIT_ASSERT_MESSAGE("=C1+C2", fValue == 11);
    //check merged cells import
    SCCOL nCol = 4;
    SCROW nRow = 1;
    pDoc->ExtendMerge(4, 1, nCol, nRow, 0, false);
    CPPUNIT_ASSERT_MESSAGE("merged cells are not imported", nCol == 5 && nRow == 2);
    //check notes import
    ScAddress aAddress(7, 2, 0);
    ScPostIt* pNote = pDoc->GetNotes(aAddress.Tab())->findByAddress(aAddress);
    CPPUNIT_ASSERT_MESSAGE("note not imported", pNote);
    CPPUNIT_ASSERT_MESSAGE("note text not imported correctly", pNote->GetText() == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Test")));
    //add additional checks here
}

}

void ScFiltersTest::testContentODS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("universal-content."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 0);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testContentImpl(pDoc);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentXLS()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("universal-content."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 1);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testContentImpl(pDoc);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentXLSX()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("universal-content."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 2);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testContentImpl(pDoc);
    xDocSh->DoClose();
}

void ScFiltersTest::testContentLotus123()
{
    const rtl::OUString aFileNameBase(RTL_CONSTASCII_USTRINGPARAM("universal-content."));
    ScDocShellRef xDocSh = loadDoc(aFileNameBase, 3);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    //testContentImpl(pDoc);
    xDocSh->DoClose();
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
