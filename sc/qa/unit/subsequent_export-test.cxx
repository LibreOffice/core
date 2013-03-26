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
#include <sfx2/frame.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>

#include <unotools/tempfile.hxx>
#include <comphelper/storagehelper.hxx>

#define CALC_DEBUG_OUTPUT 0
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
#include "svx/svdpage.hxx"

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
    const char* pName; const char* pFilterName; const char* pTypeName; unsigned int nFormatType;
};

FileFormat aFileFormats[] = {
    { "ods" , "calc8", "", ODS_FORMAT_TYPE },
    { "xls" , "MS Excel 97", "calc_MS_EXCEL_97", XLS_FORMAT_TYPE },
    { "xlsx", "Calc MS Excel 2007 XML" , "MS Excel 2007 XML", XLSX_FORMAT_TYPE },
    { "123" , "Lotus", "calc_Lotus", LOTUS123_FORMAT_TYPE }
};

}

class ScExportTest : public test::BootstrapFixture
{
public:
    ScExportTest();

    virtual void setUp();
    virtual void tearDown();

    ScDocShellRef saveAndReload( ScDocShell*, const rtl::OUString&, const rtl::OUString&, const rtl::OUString&, sal_uLong );
    ScDocShellRef saveAndReloadPassword( ScDocShell*, const rtl::OUString&, const rtl::OUString&, const rtl::OUString&, sal_uLong );

    void test();
    void testPasswordExport();
    void testConditionalFormatExportXLSX();
    void testMiscRowHeightExport();
    void testRotatedFlippedShapes();

    CPPUNIT_TEST_SUITE(ScExportTest);
    CPPUNIT_TEST(test);
#if !defined(MACOSX) && !defined(DRAGONFLY) && !defined(WNT)
    CPPUNIT_TEST(testPasswordExport);
#endif
    CPPUNIT_TEST(testConditionalFormatExportXLSX);
    CPPUNIT_TEST(testMiscRowHeightExport);
    CPPUNIT_TEST(testRotatedFlippedShapes);
    CPPUNIT_TEST_SUITE_END();

private:
    rtl::OUString m_aBaseString;
    uno::Reference<uno::XInterface> m_xCalcComponent;

    ScDocShellRef load(
        const OUString& rURL, const OUString& rFilter, const OUString &rUserData,
        const OUString& rTypeName, sal_Int32 nFormat, sal_uLong nFormatType,
        const OUString* pPassword = NULL );

    ScDocShellRef saveAndReload( ScDocShell* pShell, sal_Int32 nFormat );
    ScDocShellRef loadDocument( const rtl::OUString& rFileNameBase, sal_Int32 nFormat );
    void createFileURL( const rtl::OUString& aFileBase, const rtl::OUString& rFileExtension, rtl::OUString& rFilePath);
    void createCSVPath(const rtl::OUString& rFileBase, rtl::OUString& rCSVPath);
};

void ScExportTest::createFileURL(const rtl::OUString& aFileBase, const rtl::OUString& aFileExtension, rtl::OUString& rFilePath)
{
    rtl::OUString aSep("/");
    rtl::OUStringBuffer aBuffer( getSrcRootURL() );
    aBuffer.append(m_aBaseString).append(aSep).append(aFileExtension);
    aBuffer.append(aSep).append(aFileBase).append(aFileExtension);
    rFilePath = aBuffer.makeStringAndClear();
}

void ScExportTest::createCSVPath(const rtl::OUString& aFileBase, rtl::OUString& rCSVPath)
{
    rtl::OUStringBuffer aBuffer(getSrcRootPath());
    aBuffer.append(m_aBaseString).append(rtl::OUString("/contentCSV/"));
    aBuffer.append(aFileBase).append(rtl::OUString("csv"));
    rCSVPath = aBuffer.makeStringAndClear();
}

ScDocShellRef ScExportTest::saveAndReloadPassword(ScDocShell* pShell, const rtl::OUString &rFilter,
    const rtl::OUString &rUserData, const rtl::OUString& rTypeName, sal_uLong nFormatType)
{
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    SfxMedium aStoreMedium( aTempFile.GetURL(), STREAM_STD_WRITE );
    sal_uInt32 nExportFormat = 0;
    if (nFormatType == ODS_FORMAT_TYPE)
        nExportFormat = SFX_FILTER_EXPORT | SFX_FILTER_USESOPTIONS;
    SfxFilter* pExportFilter = new SfxFilter(
        rFilter,
        rtl::OUString(), nFormatType, nExportFormat, rTypeName, 0, rtl::OUString(),
        rUserData, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/scalc*")) );
    pExportFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
    aStoreMedium.SetFilter(pExportFilter);
    SfxItemSet* pExportSet = aStoreMedium.GetItemSet();
    uno::Sequence< beans::NamedValue > aEncryptionData = comphelper::OStorageHelper::CreatePackageEncryptionData( rtl::OUString("test") );
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
    return load(aTempFile.GetURL(), rFilter, rUserData, rTypeName, nFormat, nFormatType, &aPass);
}

ScDocShellRef ScExportTest::saveAndReload(ScDocShell* pShell, const rtl::OUString &rFilter,
    const rtl::OUString &rUserData, const rtl::OUString& rTypeName, sal_uLong nFormatType)
{

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    SfxMedium aStoreMedium( aTempFile.GetURL(), STREAM_STD_WRITE );
    sal_uInt32 nExportFormat = 0;
    if (nFormatType == ODS_FORMAT_TYPE)
        nExportFormat = SFX_FILTER_EXPORT | SFX_FILTER_USESOPTIONS;
    SfxFilter* pExportFilter = new SfxFilter(
        rFilter,
        rtl::OUString(), nFormatType, nExportFormat, rTypeName, 0, rtl::OUString(),
        rUserData, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/scalc*")) );
    pExportFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
    aStoreMedium.SetFilter(pExportFilter);
    pShell->DoSaveAs( aStoreMedium );
    pShell->DoClose();

    //std::cout << "File: " << aTempFile.GetURL() << std::endl;

    sal_uInt32 nFormat = 0;
    if (nFormatType == ODS_FORMAT_TYPE)
        nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;

    return load(aTempFile.GetURL(), rFilter, rUserData, rTypeName, nFormat, nFormatType);
}

ScDocShellRef ScExportTest::load(
    const OUString& rURL, const OUString& rFilter, const OUString &rUserData,
    const OUString& rTypeName, sal_Int32 nFormat, sal_uLong nFormatType, const OUString* pPassword )
{
    SfxFilter* pFilter = new SfxFilter(
        rFilter,
        rtl::OUString(), nFormatType, nFormat, rTypeName, 0, rtl::OUString(),
        rUserData, OUString("private:factory/scalc*"));
    pFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    ScDocShellRef xDocShRef = new ScDocShell;
    xDocShRef->GetDocument()->EnableUserInteraction(false);
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ);
    pSrcMed->SetFilter(pFilter);
    pSrcMed->UseInteractionHandler(false);
    if (pPassword)
    {
        SfxItemSet* pSet = pSrcMed->GetItemSet();
        pSet->Put(SfxStringItem(SID_PASSWORD, *pPassword));
    }
    if (!xDocShRef->DoLoad(pSrcMed))
    {
        xDocShRef->DoClose();
        // load failed.
        xDocShRef.Clear();
    }

    return xDocShRef;
}

ScDocShellRef ScExportTest::saveAndReload( ScDocShell* pShell, sal_Int32 nFormat )
{
    rtl::OUString aFileExtension(aFileFormats[nFormat].pName, strlen(aFileFormats[nFormat].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[nFormat].pFilterName, strlen(aFileFormats[nFormat].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFilterType(aFileFormats[nFormat].pTypeName, strlen(aFileFormats[nFormat].pTypeName), RTL_TEXTENCODING_UTF8);
    ScDocShellRef xDocSh = saveAndReload(pShell, aFilterName, rtl::OUString(), aFilterType, aFileFormats[nFormat].nFormatType);

    CPPUNIT_ASSERT(xDocSh.Is());
    return xDocSh;
}

ScDocShellRef ScExportTest::loadDocument(const rtl::OUString& rFileName, sal_Int32 nFormat)
{
    rtl::OUString aFileExtension(aFileFormats[nFormat].pName, strlen(aFileFormats[nFormat].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[nFormat].pFilterName, strlen(aFileFormats[nFormat].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFileName;
    createFileURL( rFileName, aFileExtension, aFileName );
    rtl::OUString aFilterType(aFileFormats[nFormat].pTypeName, strlen(aFileFormats[nFormat].pTypeName), RTL_TEXTENCODING_UTF8);
    unsigned int nFormatType = aFileFormats[nFormat].nFormatType;
    unsigned int nClipboardId = nFormatType ? SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS : 0;

    return load(aFileName, aFilterName, OUString(), aFilterType, nClipboardId, nFormatType);
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
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aVal, 1.0, 1e-8);
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
    rtl::OUString aFileExtension(aFileFormats[nFormat].pName, strlen(aFileFormats[nFormat].pName), RTL_TEXTENCODING_UTF8 );
    rtl::OUString aFilterName(aFileFormats[nFormat].pFilterName, strlen(aFileFormats[nFormat].pFilterName), RTL_TEXTENCODING_UTF8) ;
    rtl::OUString aFilterType(aFileFormats[nFormat].pTypeName, strlen(aFileFormats[nFormat].pTypeName), RTL_TEXTENCODING_UTF8);
    ScDocShellRef xDocSh = saveAndReloadPassword(pShell, aFilterName, rtl::OUString(), aFilterType, aFileFormats[nFormat].nFormatType);

    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pLoadedDoc = xDocSh->GetDocument();
    double aVal = pLoadedDoc->GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aVal, 1.0, 1e-8);
}

void ScExportTest::testConditionalFormatExportXLSX()
{
    ScDocShellRef xShell = loadDocument("new_cond_format_test.", XLSX);
    CPPUNIT_ASSERT(xShell.Is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    rtl::OUString aCSVFile("new_cond_format_test.");
    rtl::OUString aCSVPath;
    createCSVPath( aCSVFile, aCSVPath );
    testCondFile(aCSVPath, pDoc, 0);
}

void ScExportTest::testMiscRowHeightExport()
{

    struct TestParam
    {
        struct RowData
        {
            SCROW nStartRow;
            SCROW nEndRow;
            SCTAB nTab;
            int nExpectedHeight;
        };
        const char* sTestDoc;
        int nImportType;
        int nExportType;
        int nRowData;
        RowData* pData;
    };

    TestParam::RowData DfltRowData[] =
    {
        { 0, 4, 0, 529 },
        { 5, 10, 0, 1058 },
        { 17, 20, 0, 1767 },
        { 1048573, 1048575, 0, 529 },
    };

    TestParam::RowData EmptyRepeatRowData[] =
    {
        { 0, 4, 0, 529 },
        { 5, 10, 0, 1058 },
        { 17, 20, 0, 1767 },
    };

    TestParam aTestValues[] =
    {
        { "miscrowheights.", XLSX, XLSX, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        { "miscrowheights.", XLSX, XLS, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        { "miscemptyrepeatedrowheights.", ODS, XLSX, SAL_N_ELEMENTS(EmptyRepeatRowData), EmptyRepeatRowData },
        { "miscemptyrepeatedrowheights.", ODS, XLS, SAL_N_ELEMENTS(EmptyRepeatRowData), EmptyRepeatRowData },
    };

    for ( unsigned int index=0; index<SAL_N_ELEMENTS(aTestValues); ++index )
    {
        OUString sFileName = OUString::createFromAscii( aTestValues[ index ].sTestDoc );
        printf("aTestValues[%d] %s\n", index, OUStringToOString( sFileName, RTL_TEXTENCODING_UTF8 ).getStr() );
        int nImportType =  aTestValues[ index ].nImportType;
        int nExportType =  aTestValues[ index ].nExportType;
        ScDocShellRef xShell = loadDocument( sFileName, nImportType );
        CPPUNIT_ASSERT(xShell.Is());

        ScDocShellRef xDocSh = saveAndReload(&(*xShell), nExportType );
        CPPUNIT_ASSERT(xDocSh.Is());

        ScDocument* pDoc = xDocSh->GetDocument();

        for (int i=0; i<aTestValues[ index ].nRowData; ++i)
        {
            SCROW nRow = aTestValues[ index ].pData[ i].nStartRow;
            SCROW nEndRow = aTestValues[ index ].pData[ i ].nEndRow;
            SCTAB nTab = aTestValues[ index ].pData[ i ].nTab;
            int nExpectedHeight = aTestValues[ index ].pData[ i ].nExpectedHeight;
            for ( ; nRow <= nEndRow; ++nRow )
            {
                printf("\t checking row %d for height %d\n", nRow, nExpectedHeight );
                int nHeight = sc::TwipsToHMM( pDoc->GetRowHeight(nRow, nTab, false) );
                CPPUNIT_ASSERT_EQUAL(nExpectedHeight, nHeight);
            }
        }
    }
}

void ScExportTest::testRotatedFlippedShapes()
{
    ScDocShellRef xShell = loadDocument("rotflipshapes.", ODS);
    CPPUNIT_ASSERT(xShell.Is());
    ScDocShellRef xDocSh = saveAndReload(&(*xShell), ODS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    ScDrawLayer *pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("must have a draw layer", pDrawLayer != NULL);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("must have a draw page", pPage != NULL);
    struct TestData
    {
        long mnShapeX;
        long mnShapeY;
        long mnShapeWidth;
        long mnShapeHeight;
        SCROW startAnchorRow;
        SCCOL startAnchorCol;
        SCROW endAnchorRow;
        SCCOL endAnchorCol;
    };

    TestData testData[] =
    {
        { 1, 1356, 2194, 1255, 3, 0, 5, 0  },
        { 2194, 5771, 2195, 1205, 12, 0, 15, 1 },
        { 5154, 1382, 904, 1170, 3, 2, 5, 2 },
        { 6085, 5712, 903, 1120, 12, 2, 15, 3 },
        { 9367, 1906, 1382, 772, 4, 4, 5, 4 },
        { 10557, 5732, 1383, 745, 12, 4, 14, 5 },
        { 14106, 1674, 957, 957, 3, 6, 5, 6 },
        { 15087, 5447, 956, 932, 12, 6, 14, 7 },
        { 18087, 2050, 904, 1010, 4, 8, 6, 8 },
        { 19122, 5569, 904, 985, 12, 8, 14, 8 }
    };

    int nCount( pPage->GetObjCount() );
    int nData = SAL_N_ELEMENTS(testData);
    CPPUNIT_ASSERT_MESSAGE("must have test data for all objects", nCount == nData );

    for (int i = 0; i < nData; ++i )

    {
        SdrObject* pObj = pPage->GetObj(i);
        CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
        ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj, false);
        Point aObjPos( pObj->GetLogicRect().TopLeft() );

       long nTol = 20; // 20hmm
       printf( "comparing object[%d] pos ( %ld, %ld ) with expected ( %ld, %ld ) - with tolerance %ld\n",
            i, aObjPos.X(), aObjPos.Y(), testData[ i ].mnShapeX, testData[ i ].mnShapeY, nTol );
       CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aObjPos.X(), testData[ i ].mnShapeX, nTol  ) );
       CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance( aObjPos.Y(), testData[ i ].mnShapeY, nTol  ) );

       printf( "comparing object[%d] Width, Height ( %ld, %ld ) with expected ( %ld, %ld ) - with tolerance %ld\n",
             i, pObj->GetLogicRect().GetWidth(), pObj->GetLogicRect().GetHeight(),
             testData[ i ].mnShapeWidth, testData[ i ].mnShapeHeight, nTol );
       CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance(pObj->GetLogicRect().GetWidth(), testData[ i ].mnShapeWidth, nTol  ) );
       CPPUNIT_ASSERT_EQUAL( true, testEqualsWithTolerance(pObj->GetLogicRect().GetHeight(), testData[ i ].mnShapeHeight, nTol  ) );

       printf( "comparing object[%d] anchor start ( %" SAL_PRIdINT32 ", %d ) with expected ( %" SAL_PRIdINT32 ", %d ) \n",
             i, pData->maStart.Row(), pData->maStart.Col(),  testData[ i ].startAnchorRow, testData[ i ].startAnchorCol );

       CPPUNIT_ASSERT_EQUAL( pData->maStart.Row(), testData[ i ].startAnchorRow );
       CPPUNIT_ASSERT_EQUAL( pData->maStart.Col(), testData[ i ].startAnchorCol );

       printf( "comparing object[%d] anchor end ( %" SAL_PRIdINT32 ", %d ) with expected ( %" SAL_PRIdINT32 ", %d ) \n",
             i, pData->maEnd.Row(), pData->maEnd.Col(),  testData[ i ].endAnchorRow, testData[ i ].endAnchorCol );
       CPPUNIT_ASSERT_EQUAL( pData->maEnd.Row(), testData[ i ].endAnchorRow );
       CPPUNIT_ASSERT_EQUAL( pData->maEnd.Col(), testData[ i ].endAnchorCol );
    }
}
ScExportTest::ScExportTest()
      : m_aBaseString(RTL_CONSTASCII_USTRINGPARAM("/sc/qa/unit/data"))
{
}

void ScExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.SpreadsheetDocument")));
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
