/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <officecfg/Office/Common.hxx>
#include <sal/config.h>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <config_features.h>

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>

#include "helper/qahelper.hxx"
#include "helper/xpath.hxx"
#include "helper/shared_test_impl.hxx"

#include <userdat.hxx>
#include <docsh.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <scitems.hxx>
#include <document.hxx>
#include <cellform.hxx>
#include <formulacell.hxx>
#include <tokenarray.hxx>
#include <editutil.hxx>
#include <scopetools.hxx>
#include <cellvalue.hxx>
#include <docfunc.hxx>
#include <generalfunction.hxx>
#include <postit.hxx>
#include <tokenstringcontext.hxx>
#include <chgtrack.hxx>
#include <validat.hxx>
#include <attrib.hxx>
#include <global.hxx>
#include <scmod.hxx>

#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdograf.hxx>
#include <tabprotection.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/section.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/colritem.hxx>
#include <formula/grammar.hxx>
#include <unotools/useroptions.hxx>
#include <tools/datetime.hxx>
#include <svl/zformat.hxx>

#include <test/xmltesttools.hxx>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <comphelper/storagehelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScExportTest : public ScBootstrapFixture, public XmlTestTools
{
protected:
    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;
public:
    ScExportTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    ScDocShellRef saveAndReloadPassword( ScDocShell*, const OUString&, const OUString&, const OUString&, SfxFilterFlags );

    void test();
    void testTdf111876();
    void testPasswordExportODS();
    void testConditionalFormatExportODS();
    void testConditionalFormatExportXLSX();
    void testTdf99856_dataValidationTest();
    void testProtectionKeyODS_UTF16LErtlSHA1();
    void testProtectionKeyODS_UTF8SHA1();
    void testProtectionKeyODS_UTF8SHA256ODF12();
    void testProtectionKeyODS_UTF8SHA256W3C();
    void testProtectionKeyODS_XL_SHA1();
    void testColorScaleExportODS();
    void testColorScaleExportXLSX();
    void testDataBarExportODS();
    void testDataBarExportXLSX();
    void testConditionalFormatRangeListXLSX();
    void testConditionalFormatContainsTextXLSX();
    void testMiscRowHeightExport();
    void testNamedRangeBugfdo62729();
    void testBuiltinRangesXLSX();
    void testRichTextExportODS();
    void testRichTextCellFormatXLSX();
    void testFormulaRefSheetNameODS();

    void testCellValuesExportODS();
    void testCellNoteExportODS();
    void testCellNoteExportXLS();
    void testFormatExportODS();

    void testCommentExportXLSX();
#if HAVE_MORE_FONTS
    void testCustomColumnWidthExportXLSX();
#endif
    void testXfDefaultValuesXLSX();
    void testColumnWidthResaveXLSX();
#if HAVE_MORE_FONTS
    void testColumnWidthExportFromODStoXLSX();
#endif
    void testOutlineExportXLSX();
    void testHiddenEmptyRowsXLSX();
    void testAllRowsHiddenXLSX();
    void testLandscapeOrientationXLSX();

    void testInlineArrayXLS();
    void testEmbeddedChartODS();
    void testEmbeddedChartXLS();
    void testCellAnchoredGroupXLS();

    void testFormulaReferenceXLS();
    void testSheetProtectionXLSX();
    void testSheetProtectionXLSB();

    void testCellBordersXLS();
    void testCellBordersXLSX();
    void testBordersExchangeXLSX();
    void testTrackChangesSimpleXLSX();
    void testSheetTabColorsXLSX();

    void testSharedFormulaExportXLS();
    void testSharedFormulaExportXLSX();
    void testSharedFormulaStringResultExportXLSX();

    void testFunctionsExcel2010( sal_uLong nFormatType );
    void testFunctionsExcel2010XLSX();
    void testFunctionsExcel2010XLS();
    void testFunctionsExcel2010ODS();

    void testCeilingFloor( sal_uLong nFormatType );
    void testCeilingFloorXLSX();
    void testCeilingFloorODSToXLSX();
    void testCeilingFloorXLS();
    void testCeilingFloorODS();

    void testCustomXml();

#if !defined _WIN32
    void testRelativePathsODS();
#endif
    void testSheetProtectionODS();

    void testSwappedOutImageExport();
    void testLinkedGraphicRT();
    void testImageWithSpecialID();

#if !defined _WIN32
    void testSupBookVirtualPathXLS();
#endif
    void testAbsNamedRangeHTML();
    void testSheetLocalRangeNameXLS();
    void testRelativeNamedExpressionsXLS();
    void testSheetTextBoxHyperlinkXLSX();
    void testFontSizeXLSX();
    void testSheetCharacterKerningSpaceXLSX();
    void testSheetCondensedCharacterSpaceXLSX();
    void testTextUnderlineColorXLSX();
    void testSheetRunParagraphPropertyXLSX();
    void testHiddenShapeXLSX();
    void testHyperlinkXLSX();
    void testMoveCellAnchoredShapesODS();
    void testMatrixMultiplicationXLSX();
    void testPreserveTextWhitespaceXLSX();
    void testPreserveTextWhitespace2XLSX();
    void testTextDirectionXLSX();

    void testRefStringXLSX();
    void testRefStringConfigXLSX();
    void testRefStringUnspecified();
    void testHeaderImageODS();

    void testTdf88657ODS();
    void testEscapeCharInNumberFormatXLSX();
    void testNatNumInNumberFormatXLSX();
    void testExponentWithoutSignFormatXLSX();
    void testExtendedLCIDXLSX();

    void testHiddenRepeatedRowsODS();
    void testHyperlinkTargetFrameODS();
    void testOpenDocumentAsReadOnly();

    CPPUNIT_TEST_SUITE(ScExportTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(testTdf111876);
    CPPUNIT_TEST(testPasswordExportODS);
    CPPUNIT_TEST(testConditionalFormatExportODS);
    CPPUNIT_TEST(testConditionalFormatExportXLSX);
    CPPUNIT_TEST(testTdf99856_dataValidationTest);
    CPPUNIT_TEST(testProtectionKeyODS_UTF16LErtlSHA1);
    CPPUNIT_TEST(testProtectionKeyODS_UTF8SHA1);
    CPPUNIT_TEST(testProtectionKeyODS_UTF8SHA256ODF12);
    CPPUNIT_TEST(testProtectionKeyODS_UTF8SHA256W3C);
    CPPUNIT_TEST(testProtectionKeyODS_XL_SHA1);
    CPPUNIT_TEST(testColorScaleExportODS);
    CPPUNIT_TEST(testColorScaleExportXLSX);
    CPPUNIT_TEST(testDataBarExportODS);
    CPPUNIT_TEST(testDataBarExportXLSX);
    CPPUNIT_TEST(testConditionalFormatRangeListXLSX);
    CPPUNIT_TEST(testConditionalFormatContainsTextXLSX);
    CPPUNIT_TEST(testMiscRowHeightExport);
    CPPUNIT_TEST(testNamedRangeBugfdo62729);
    CPPUNIT_TEST(testBuiltinRangesXLSX);
    CPPUNIT_TEST(testRichTextExportODS);
    CPPUNIT_TEST(testRichTextCellFormatXLSX);
    CPPUNIT_TEST(testFormulaRefSheetNameODS);
    CPPUNIT_TEST(testCellValuesExportODS);
    CPPUNIT_TEST(testCellNoteExportODS);
    CPPUNIT_TEST(testCellNoteExportXLS);
    CPPUNIT_TEST(testFormatExportODS);

    CPPUNIT_TEST(testCommentExportXLSX);
#if HAVE_MORE_FONTS
    CPPUNIT_TEST(testCustomColumnWidthExportXLSX);
#endif
    CPPUNIT_TEST(testXfDefaultValuesXLSX);
    CPPUNIT_TEST(testColumnWidthResaveXLSX);
#if HAVE_MORE_FONTS
    CPPUNIT_TEST(testColumnWidthExportFromODStoXLSX);
#endif
    CPPUNIT_TEST(testOutlineExportXLSX);
    CPPUNIT_TEST(testHiddenEmptyRowsXLSX);
    CPPUNIT_TEST(testAllRowsHiddenXLSX);
    CPPUNIT_TEST(testLandscapeOrientationXLSX);
    CPPUNIT_TEST(testInlineArrayXLS);
    CPPUNIT_TEST(testEmbeddedChartODS);
    CPPUNIT_TEST(testEmbeddedChartXLS);
    CPPUNIT_TEST(testCellAnchoredGroupXLS);

    CPPUNIT_TEST(testFormulaReferenceXLS);
    CPPUNIT_TEST(testSheetProtectionXLSX);
    CPPUNIT_TEST(testSheetProtectionXLSB);
    CPPUNIT_TEST(testCellBordersXLS);
    CPPUNIT_TEST(testCellBordersXLSX);
    CPPUNIT_TEST(testBordersExchangeXLSX);
    CPPUNIT_TEST(testTrackChangesSimpleXLSX);
    CPPUNIT_TEST(testSheetTabColorsXLSX);
    CPPUNIT_TEST(testSharedFormulaExportXLS);
    CPPUNIT_TEST(testSharedFormulaExportXLSX);
    CPPUNIT_TEST(testSharedFormulaStringResultExportXLSX);
    CPPUNIT_TEST(testFunctionsExcel2010XLSX);
    CPPUNIT_TEST(testFunctionsExcel2010XLS);
    CPPUNIT_TEST(testFunctionsExcel2010ODS);
    CPPUNIT_TEST(testCeilingFloorXLSX);
    CPPUNIT_TEST(testCeilingFloorODSToXLSX);
    CPPUNIT_TEST(testCeilingFloorXLS);
    CPPUNIT_TEST(testCeilingFloorODS);
    CPPUNIT_TEST(testCustomXml);
#if !defined(_WIN32)
    CPPUNIT_TEST(testRelativePathsODS);
#endif
    CPPUNIT_TEST(testSheetProtectionODS);
#if !defined(_WIN32)
    CPPUNIT_TEST(testSupBookVirtualPathXLS);
#endif
    CPPUNIT_TEST(testSwappedOutImageExport);
    CPPUNIT_TEST(testLinkedGraphicRT);
    CPPUNIT_TEST(testImageWithSpecialID);
    CPPUNIT_TEST(testPreserveTextWhitespaceXLSX);
    CPPUNIT_TEST(testPreserveTextWhitespace2XLSX);
    CPPUNIT_TEST(testAbsNamedRangeHTML);
    CPPUNIT_TEST(testSheetLocalRangeNameXLS);
    CPPUNIT_TEST(testRelativeNamedExpressionsXLS);
    CPPUNIT_TEST(testSheetTextBoxHyperlinkXLSX);
    CPPUNIT_TEST(testFontSizeXLSX);
    CPPUNIT_TEST(testSheetCharacterKerningSpaceXLSX);
    CPPUNIT_TEST(testSheetCondensedCharacterSpaceXLSX);
    CPPUNIT_TEST(testTextUnderlineColorXLSX);
    CPPUNIT_TEST(testSheetRunParagraphPropertyXLSX);
    CPPUNIT_TEST(testHiddenShapeXLSX);
    CPPUNIT_TEST(testHyperlinkXLSX);
    CPPUNIT_TEST(testMoveCellAnchoredShapesODS);
    CPPUNIT_TEST(testMatrixMultiplicationXLSX);
    CPPUNIT_TEST(testTextDirectionXLSX);

    CPPUNIT_TEST(testRefStringXLSX);
    CPPUNIT_TEST(testRefStringConfigXLSX);
    CPPUNIT_TEST(testRefStringUnspecified);
    CPPUNIT_TEST(testHeaderImageODS);

    CPPUNIT_TEST(testTdf88657ODS);
    CPPUNIT_TEST(testEscapeCharInNumberFormatXLSX);
    CPPUNIT_TEST(testNatNumInNumberFormatXLSX);
    CPPUNIT_TEST(testExponentWithoutSignFormatXLSX);
    CPPUNIT_TEST(testExtendedLCIDXLSX);

    CPPUNIT_TEST(testHiddenRepeatedRowsODS);
    CPPUNIT_TEST(testHyperlinkTargetFrameODS);
    CPPUNIT_TEST(testOpenDocumentAsReadOnly);

    CPPUNIT_TEST_SUITE_END();

private:
    void testExcelCellBorders( sal_uLong nFormatType );

    uno::Reference<uno::XInterface> m_xCalcComponent;

};

void ScExportTest::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    struct { xmlChar* pPrefix; xmlChar* pURI; } aNamespaces[] =
    {
        { BAD_CAST("w"), BAD_CAST("http://schemas.openxmlformats.org/wordprocessingml/2006/main") },
        { BAD_CAST("x"), BAD_CAST("http://schemas.openxmlformats.org/spreadsheetml/2006/main") },
        { BAD_CAST("v"), BAD_CAST("urn:schemas-microsoft-com:vml") },
        { BAD_CAST("c"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/chart") },
        { BAD_CAST("a"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/main") },
        { BAD_CAST("mc"), BAD_CAST("http://schemas.openxmlformats.org/markup-compatibility/2006") },
        { BAD_CAST("wps"), BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingShape") },
        { BAD_CAST("wpg"), BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingGroup") },
        { BAD_CAST("wp"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing") },
        { BAD_CAST("office"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:office:1.0") },
        { BAD_CAST("table"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:table:1.0") },
        { BAD_CAST("text"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:text:1.0") },
        { BAD_CAST("style"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:style:1.0") },
        { BAD_CAST("draw"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0") },
        { BAD_CAST("xlink"), BAD_CAST("http://www.w3c.org/1999/xlink") },
        { BAD_CAST("xdr"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing") },
        { BAD_CAST("x"), BAD_CAST("http://schemas.openxmlformats.org/spreadsheetml/2006/main") },
        { BAD_CAST("r"), BAD_CAST("http://schemas.openxmlformats.org/package/2006/relationships") },
        { BAD_CAST("number"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0") },
        { BAD_CAST("loext"), BAD_CAST("urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0") },
    };
    for(size_t i = 0; i < SAL_N_ELEMENTS(aNamespaces); ++i)
    {
        xmlXPathRegisterNs(pXmlXPathCtx, aNamespaces[i].pPrefix, aNamespaces[i].pURI );
    }
}

ScDocShellRef ScExportTest::saveAndReloadPassword(ScDocShell* pShell, const OUString &rFilter,
    const OUString &rUserData, const OUString& rTypeName, SfxFilterFlags nFormatType)
{
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    SfxMedium aStoreMedium( aTempFile.GetURL(), StreamMode::STD_WRITE );
    SotClipboardFormatId nExportFormat = SotClipboardFormatId::NONE;
    if (nFormatType == ODS_FORMAT_TYPE)
        nExportFormat = SotClipboardFormatId::STARCHART_8;
    std::shared_ptr<const SfxFilter> pExportFilter(new SfxFilter(
        rFilter,
        OUString(), nFormatType, nExportFormat, rTypeName, OUString(),
        rUserData, "private:factory/scalc*" ));
    const_cast<SfxFilter*>(pExportFilter.get())->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
    aStoreMedium.SetFilter(pExportFilter);
    SfxItemSet* pExportSet = aStoreMedium.GetItemSet();
    uno::Sequence< beans::NamedValue > aEncryptionData = comphelper::OStorageHelper::CreatePackageEncryptionData( "test" );
    pExportSet->Put(SfxUnoAnyItem(SID_ENCRYPTIONDATA, makeAny(aEncryptionData)));

    uno::Reference< embed::XStorage > xMedStorage = aStoreMedium.GetStorage();
    ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( xMedStorage, aEncryptionData );

    pShell->DoSaveAs( aStoreMedium );
    pShell->DoClose();

    //std::cout << "File: " << aTempFile.GetURL() << std::endl;

    SotClipboardFormatId nFormat = SotClipboardFormatId::NONE;
    if (nFormatType == ODS_FORMAT_TYPE)
        nFormat = SotClipboardFormatId::STARCALC_8;

    OUString aPass("test");
    return load(aTempFile.GetURL(), rFilter, rUserData, rTypeName, nFormatType, nFormat, SOFFICE_FILEFORMAT_CURRENT, &aPass);
}

void ScExportTest::test()
{
    ScDocShell* pShell = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    pShell->DoInitNew();

    ScDocument& rDoc = pShell->GetDocument();

    rDoc.SetValue(0,0,0, 1.0);

    ScDocShellRef xDocSh = saveAndReload(pShell, FORMAT_ODS);

    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rLoadedDoc = xDocSh->GetDocument();
    double aVal = rLoadedDoc.GetValue(0,0,0);
    ASSERT_DOUBLES_EQUAL(aVal, 1.0);
    xDocSh->DoClose();
}

void ScExportTest::testTdf111876()
 {
    // DOcument with relative path hyperlink

    ScDocShellRef xShell = loadDoc("tdf111876.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/_rels/sheet1.xml.rels", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    OUString sTarget = getXPath(pDoc, "/r:Relationships/r:Relationship", "Target");

    // Document is saved to the temporary directory, relative path should be different than original one
    CPPUNIT_ASSERT(sTarget != "../xls/bug-fixes.xls");

    xDocSh->DoClose();
}

void ScExportTest::testPasswordExportODS()
{
    ScDocShell* pShell = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    pShell->DoInitNew();

    ScDocument& rDoc = pShell->GetDocument();

    rDoc.SetValue(0, 0, 0, 1.0);

    sal_Int32 nFormat = FORMAT_ODS;
    OUString aFilterName(getFileFormats()[nFormat].pFilterName, strlen(getFileFormats()[nFormat].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFilterType(getFileFormats()[nFormat].pTypeName, strlen(getFileFormats()[nFormat].pTypeName), RTL_TEXTENCODING_UTF8);
    ScDocShellRef xDocSh = saveAndReloadPassword(pShell, aFilterName, OUString(), aFilterType, getFileFormats()[nFormat].nFormatType);

    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rLoadedDoc = xDocSh->GetDocument();
    double aVal = rLoadedDoc.GetValue(0,0,0);
    ASSERT_DOUBLES_EQUAL(aVal, 1.0);

    xDocSh->DoClose();
}

void ScExportTest::testConditionalFormatExportODS()
{
    ScDocShellRef xShell = loadDoc("new_cond_format_test_export.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVPath;
    createCSVPath( "new_cond_format_test_export.", aCSVPath );
    testCondFile(aCSVPath, &rDoc, 0);

    xDocSh->DoClose();
}

void ScExportTest::testConditionalFormatExportXLSX()
{
    ScDocShellRef xShell = loadDoc("new_cond_format_test_export.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    {
        OUString aCSVPath;
        createCSVPath( "new_cond_format_test_export.", aCSVPath );
        testCondFile(aCSVPath, &rDoc, 0);
    }
    {
        OUString aCSVPath;
        createCSVPath( "new_cond_format_test_sheet2.", aCSVPath );
        testCondFile(aCSVPath, &rDoc, 1);
    }

    xDocSh->DoClose();
}

void ScExportTest::testTdf99856_dataValidationTest()
{
    ScDocShellRef xShell = loadDoc("tdf99856_dataValidationTest.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load doc", xShell.is());

    ScDocShellRef xDocSh = saveAndReload( xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const ScValidationData* pData = rDoc.GetValidationEntry(2);
    CPPUNIT_ASSERT(pData);

    // Excel can't open corrupt file if the list is longer than 255 characters
    std::vector<ScTypedStrData> aList;
    pData->FillSelectionList(aList, ScAddress(0, 1, 1));
    CPPUNIT_ASSERT_EQUAL(size_t(18), aList.size());
    CPPUNIT_ASSERT_EQUAL(OUString("18 Missis"), aList[17].GetString());

    xDocSh->DoClose();
}

void ScExportTest::testProtectionKeyODS_UTF16LErtlSHA1()
{
    OUString const password("1012345678901234567890123456789012345678901234567890");

    ScDocShellRef xShell = loadDoc("protection-key1.", FORMAT_FODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load doc", xShell.is());

    ScDocument& rDoc = xShell->GetDocument();
    ScDocProtection *const pDocProt(rDoc.GetDocProtection());
    CPPUNIT_ASSERT(pDocProt->verifyPassword(password));
    ScTableProtection *const pTabProt(rDoc.GetTabProtection(0));
    CPPUNIT_ASSERT(pTabProt->verifyPassword(password));

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_ODS);
    xmlDocPtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
    assertXPath(pXmlDoc, "//office:spreadsheet[@table:structure-protected='true' and @table:protection-key='vbnhxyBKtPHCA1wB21zG1Oha8ZA=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");
    assertXPath(pXmlDoc, "//table:table[@table:protected='true' and @table:protection-key='vbnhxyBKtPHCA1wB21zG1Oha8ZA=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");
}

void ScExportTest::testProtectionKeyODS_UTF8SHA1()
{
    OUString const password("1012345678901234567890123456789012345678901234567890");

    ScDocShellRef xShell = loadDoc("protection-key2.", FORMAT_FODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load doc", xShell.is());

    ScDocument& rDoc = xShell->GetDocument();
    ScDocProtection *const pDocProt(rDoc.GetDocProtection());
    CPPUNIT_ASSERT(pDocProt->verifyPassword(password));
    ScTableProtection *const pTabProt(rDoc.GetTabProtection(0));
    CPPUNIT_ASSERT(pTabProt->verifyPassword(password));

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_ODS);
    xmlDocPtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
    assertXPath(pXmlDoc, "//office:spreadsheet[@table:structure-protected='true' and @table:protection-key='nLHas0RIwepGDaH4c2hpyIUvIS8=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");
    assertXPath(pXmlDoc, "//table:table[@table:protected='true' and @table:protection-key='nLHas0RIwepGDaH4c2hpyIUvIS8=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");
}

void ScExportTest::testProtectionKeyODS_UTF8SHA256ODF12()
{
    OUString const password("1012345678901234567890123456789012345678901234567890");

    ScDocShellRef xShell = loadDoc("protection-key3.", FORMAT_FODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load doc", xShell.is());

    ScDocument& rDoc = xShell->GetDocument();
    ScDocProtection *const pDocProt(rDoc.GetDocProtection());
    CPPUNIT_ASSERT(pDocProt->verifyPassword(password));
    ScTableProtection *const pTabProt(rDoc.GetTabProtection(0));
    CPPUNIT_ASSERT(pTabProt->verifyPassword(password));

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_ODS);
    xmlDocPtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
    assertXPath(pXmlDoc, "//office:spreadsheet[@table:structure-protected='true' and @table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");
    assertXPath(pXmlDoc, "//table:table[@table:protected='true' and @table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");
}

void ScExportTest::testProtectionKeyODS_UTF8SHA256W3C()
{
    OUString const password("1012345678901234567890123456789012345678901234567890");

    ScDocShellRef xShell = loadDoc("protection-key4.", FORMAT_FODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load doc", xShell.is());

    ScDocument& rDoc = xShell->GetDocument();
    ScDocProtection *const pDocProt(rDoc.GetDocProtection());
    CPPUNIT_ASSERT(pDocProt->verifyPassword(password));
    ScTableProtection *const pTabProt(rDoc.GetTabProtection(0));
    CPPUNIT_ASSERT(pTabProt->verifyPassword(password));

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_ODS);
    xmlDocPtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
    assertXPath(pXmlDoc, "//office:spreadsheet[@table:structure-protected='true' and @table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");
    assertXPath(pXmlDoc, "//table:table[@table:protected='true' and @table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");
}

void ScExportTest::testProtectionKeyODS_XL_SHA1()
{
    OUString const password("1012345678901234567890123456789012345678901234567890");

    ScDocShellRef xShell = loadDoc("protection-key5.", FORMAT_FODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load doc", xShell.is());

    ScDocument& rDoc = xShell->GetDocument();
    ScDocProtection *const pDocProt(rDoc.GetDocProtection());
    CPPUNIT_ASSERT(pDocProt->verifyPassword(password));
    ScTableProtection *const pTabProt(rDoc.GetTabProtection(0));
    CPPUNIT_ASSERT(pTabProt->verifyPassword(password));

    // we can't assume that the user entered the password; check that we
    // round-trip the password as-is
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_ODS);
    xmlDocPtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
    assertXPath(pXmlDoc, "//office:spreadsheet[@table:structure-protected='true' and @table:protection-key='OX3WkEe79fv1PE+FUmfOLdwVoqI=' and @table:protection-key-digest-algorithm='http://docs.oasis-open.org/office/ns/table/legacy-hash-excel' and @loext:protection-key-digest-algorithm-2='http://www.w3.org/2000/09/xmldsig#sha1']");
    assertXPath(pXmlDoc, "//table:table[@table:protected='true' and @table:protection-key='OX3WkEe79fv1PE+FUmfOLdwVoqI=' and @table:protection-key-digest-algorithm='http://docs.oasis-open.org/office/ns/table/legacy-hash-excel' and @loext:protection-key-digest-algorithm-2='http://www.w3.org/2000/09/xmldsig#sha1']");
}

void ScExportTest::testColorScaleExportODS()
{
    ScDocShellRef xShell = loadDoc("colorscale.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    testColorScale2Entry_Impl(rDoc);
    testColorScale3Entry_Impl(rDoc);

    xDocSh->DoClose();
}

void ScExportTest::testColorScaleExportXLSX()
{
    ScDocShellRef xShell = loadDoc("colorscale.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    testColorScale2Entry_Impl(rDoc);
    testColorScale3Entry_Impl(rDoc);

    xDocSh->DoClose();
}

void ScExportTest::testDataBarExportODS()
{
    ScDocShellRef xShell = loadDoc("databar.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    testDataBar_Impl(rDoc);

    xDocSh->DoClose();
}

void ScExportTest::testFormatExportODS()
{
    ScDocShellRef xShell = loadDoc("formats.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    testFormats(this, &rDoc, FORMAT_ODS);

    xDocSh->DoClose();
}

void ScExportTest::testCommentExportXLSX()
{
    //tdf#104729 FILESAVE OpenOffice do not save author of the comment during export to .xlsx
    ScDocShellRef xShell = loadDoc("comment.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    const xmlDocPtr pComments = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/comments1.xml");
    CPPUNIT_ASSERT(pComments);

    assertXPath(pComments, "/x:comments/x:authors/x:author[1]", "BAKO");
    assertXPath(pComments, "/x:comments/x:authors/x:author", 1);

    assertXPath(pComments, "/x:comments/x:commentList/x:comment/x:text/x:r/x:t", "Komentarz");

    const xmlDocPtr pVmlDrawing = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/vmlDrawing1.vml");
    CPPUNIT_ASSERT(pVmlDrawing);

    //assertXPath(pVmlDrawing, "/xml/v:shapetype", "coordsize", "21600,21600");
    assertXPath(pVmlDrawing, "/xml/v:shapetype", "spt", "202");
    assertXPath(pVmlDrawing, "/xml/v:shapetype/v:stroke", "joinstyle", "miter");
    const OUString sShapeTypeId = "#" + getXPath(pVmlDrawing, "/xml/v:shapetype", "id");

    assertXPath(pVmlDrawing, "/xml/v:shape", "type", sShapeTypeId);
    assertXPath(pVmlDrawing, "/xml/v:shape/v:shadow", "color", "black");
    assertXPath(pVmlDrawing, "/xml/v:shape/v:shadow", "obscured", "t");
}

#if HAVE_MORE_FONTS
void ScExportTest::testCustomColumnWidthExportXLSX()
{
    //tdf#100946 FILESAVE Excel on OS X ignored column widths in XLSX last saved by LO
    ScDocShellRef xShell = loadDoc("custom_column_width.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocPtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // First column, has everything default (width in Calc: 1280)
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "customWidth", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "min", "1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "max", "1");

    // Second column, has custom width (width in Calc: 1225)
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "customWidth", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "min", "2");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "max", "2");

    // Third column, has everything default (width in Calc: 1280)
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "customWidth", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "min", "3");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "max", "3");

    // Fourth column has custom width. Columns from 4 to 7 are hidden
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "customWidth", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "min", "4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "max", "4");

    // 5th column has custom width. Columns from 4 to 7 are hidden
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "customWidth", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "min", "5");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "max", "5");

    // 6th and 7th columns has default width and it are hidden
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "customWidth", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "min", "6");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "max", "7");

    // 8th column has everything default
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "customWidth", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "min", "8");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "max", "8");

    // 9th column has custom width
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "customWidth", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "min", "9");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "max", "9");

    // Rest of columns are default
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "customWidth", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "min", "10");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "max", "1025");

    // We expected that exactly 9 unique Nodes will be produced
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col", 9);

    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "customFormat", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "customHeight", "false");
}
#endif

void ScExportTest::testXfDefaultValuesXLSX()
{
    //tdf#70565 FORMATTING: User Defined Custom Formatting is not applied during importing XLSX documents
    ScDocShellRef xShell = loadDoc("xf_default_values.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocPtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/styles.xml");
    CPPUNIT_ASSERT(pSheet);

    // cellStyleXfs don't need xfId, so we need to make sure it is not saved
    assertXPathNoAttribute(pSheet, "/x:styleSheet/x:cellStyleXfs/x:xf[1]", "xfId");

    // Because numFmtId fontId fillId borderId xfId are not existing during import
    // it should be created during export, with values set to "0"
    assertXPath(pSheet, "/x:styleSheet/x:cellXfs/x:xf[1]", "xfId", "0");
    assertXPath(pSheet, "/x:styleSheet/x:cellXfs/x:xf[2]", "xfId", "0");
    assertXPath(pSheet, "/x:styleSheet/x:cellXfs/x:xf[3]", "xfId", "0");
    assertXPath(pSheet, "/x:styleSheet/x:cellXfs/x:xf[4]", "xfId", "0");

    // We expected that exactly 15 cellXfs:xf Nodes will be produced
    assertXPath(pSheet, "/x:styleSheet/x:cellXfs/x:xf", 14);
}

void ScExportTest::testColumnWidthResaveXLSX()
{
    // tdf#91475 FILESAVE: Column width is not preserved in XLSX / after round trip.
    // Test if after resave .xlsx file, columns width is identical with previous one
    ScDocShellRef xShell = loadDoc("different-column-width-excel2010.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocPtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // In original Excel document the width is "24"
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "width", "24");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "customWidth", "true");

    // In original Excel document the width is "12"
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "width", "12");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "customWidth", "true");

    // In original Excel document the width is "6"
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "width", "6");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "customWidth", "true");

    // In original Excel document the width is "1"
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "width", "1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "customWidth", "true");

    // In original Excel document the width is "250"
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "width", "250");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "customWidth", "true");

    // The last column [6] is not existing in Excel sheet, and it is added only by LibreOffice.
    // This column width is default and it is depended on operating system.

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col", 6);
}

#if HAVE_MORE_FONTS
void ScExportTest::testColumnWidthExportFromODStoXLSX()
{
    // tdf#91475 FILESAVE: Column width is not preserved in XLSX / after round trip.
    // Test if after export .ods to .xlsx format, displayed columns width
    // is identical with previous (.ods) one

    ScDocShellRef xShell = loadDoc("different-column-width.", FORMAT_ODS);

    CPPUNIT_ASSERT( xShell.is() );

    ScDocument& rOdsDoc = xShell->GetDocument();

    // Col 1, Tab 0 (Column width 2.00 in)
    sal_uInt16 nExpectedColumn0Width = rOdsDoc.GetColWidth(static_cast<SCCOL>(0), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL( static_cast< sal_uInt16 >( 2880 ), nExpectedColumn0Width );

    // Col 2, Tab 0 (Column width 1.00 in)
    sal_uInt16 nExpectedColumn1Width = rOdsDoc.GetColWidth(static_cast<SCCOL>(1), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL( static_cast< sal_uInt16 >( 1440 ), nExpectedColumn1Width );

    // Col 3, Tab 0 (Column width 0.50 in)
    sal_uInt16 nExpectedColumn2Width = rOdsDoc.GetColWidth(static_cast<SCCOL>(2), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL( static_cast< sal_uInt16 >( 720 ), nExpectedColumn2Width );

    // Col 4, Tab 0 (Column width 0.25 in)
    sal_uInt16 nExpectedColumn3Width = rOdsDoc.GetColWidth(static_cast<SCCOL>(3), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL( static_cast< sal_uInt16 >( 360 ), nExpectedColumn3Width  );

    // Col 5, Tab 0 (Column width 13.57 in)
    sal_uInt16 nExpectedColumn4Width = rOdsDoc.GetColWidth(static_cast<SCCOL>(4), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL( static_cast< sal_uInt16 >( 19539 ), nExpectedColumn4Width );

    // Export to .xlsx and compare column width with the .ods
    // We expect that column width from .ods will be exactly the same as imported from .xlsx

    ScDocShellRef xXlsxDocSh = saveAndReload( xShell.get(), FORMAT_XLSX );
    CPPUNIT_ASSERT( xXlsxDocSh.is() );

    ScDocument& rDoc = xXlsxDocSh->GetDocument();

    // Col 1, Tab 0
    sal_uInt16 nCalcWidth;
    nCalcWidth = rDoc.GetColWidth(static_cast<SCCOL>(0), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL( nExpectedColumn0Width, nCalcWidth );

    // Col 2, Tab 0
    nCalcWidth = rDoc.GetColWidth(static_cast<SCCOL>(1), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL( nExpectedColumn1Width, nCalcWidth );

    // Col 3, Tab 0
    nCalcWidth = rDoc.GetColWidth(static_cast<SCCOL>(2), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL( nExpectedColumn2Width, nCalcWidth );

    // Col 4, Tab 0
    nCalcWidth = rDoc.GetColWidth(static_cast<SCCOL>(3), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL( nExpectedColumn3Width, nCalcWidth );

    // Col 5, Tab 0
    nCalcWidth = rDoc.GetColWidth(static_cast<SCCOL>(4), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL( nExpectedColumn4Width, nCalcWidth );

    xXlsxDocSh->DoClose();
}
#endif

void ScExportTest::testOutlineExportXLSX()
{
    //tdf#100347 FILESAVE FILEOPEN after exporting to .xlsx format grouping are lost
    //tdf#51524  FILESAVE .xlsx and.xls looses width information for hidden/collapsed grouped columns
    ScDocShellRef xShell = loadDoc("outline.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocPtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // Maximum Outline Row is 4 for this document
    assertXPath(pSheet, "/x:worksheet/x:sheetFormatPr", "outlineLevelRow", "4");
    // Maximum Outline Column is 4 for this document
    assertXPath(pSheet, "/x:worksheet/x:sheetFormatPr", "outlineLevelCol", "4");

    // First XML node, creates two columns (from min=1 to max=2)
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "outlineLevel", "1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "min", "1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "max", "2");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "outlineLevel", "2");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "min", "3");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "max", "3");

    // Column 4 has custom width and it is hidden. We need to make sure that it is created
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "outlineLevel", "2");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "min", "4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "max", "4");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "outlineLevel", "3");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "min", "5");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "max", "6");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "outlineLevel", "4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "min", "7");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "max", "7");

    // Column 8 has custom width and it is hidden. We need to make sure that it is created
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "outlineLevel", "4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "min", "8");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[6]", "max", "8");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "outlineLevel", "4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "min", "9");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[7]", "max", "19");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "outlineLevel", "3");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "collapsed", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "min", "20");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[8]", "max", "20");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "outlineLevel", "3");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "min", "21");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[9]", "max", "21");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[10]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[10]", "outlineLevel", "2");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[10]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[10]", "min", "22");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[10]", "max", "23");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[11]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[11]", "outlineLevel", "1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[11]", "collapsed", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[11]", "min", "24");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[11]", "max", "24");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[12]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[12]", "outlineLevel", "1");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[12]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[12]", "min", "25");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[12]", "max", "26");

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[13]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[13]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[13]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[13]", "min", "27");

    // We expected that exactly 13 unique Nodes will be produced
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col", 13);

    // First row is empty and default so it is not written into XML file
    // so we need to save 29 rows, as it provides information about outLineLevel
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "r", "2");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "outlineLevel", "1");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "r", "3");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "outlineLevel", "2");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "r", "4");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "outlineLevel", "2");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "r", "5");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "outlineLevel", "3");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[5]", "r", "6");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[5]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[5]", "outlineLevel", "3");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[5]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]", "r", "7");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]", "outlineLevel", "4");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[7]", "r", "8");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[7]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[7]", "outlineLevel", "4");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[7]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[8]", "r", "9");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[8]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[8]", "outlineLevel", "4");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[8]", "collapsed", "false");
    // Next rows are the same as the previous one but it needs to bre preserved,
    // as they contain information about outlineLevel
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[20]", "r", "21");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[20]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[20]", "outlineLevel", "4");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[20]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]", "r", "22");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]", "outlineLevel", "3");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]", "collapsed", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[22]", "r", "23");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[22]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[22]", "outlineLevel", "3");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[22]", "collapsed", "false");

    // We expected that exactly 29 Row Nodes will be produced
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row", 29);
}

void ScExportTest::testAllRowsHiddenXLSX()
{
    ScDocShellRef xOrigDocSh = loadDoc("tdf105840_allRowsHidden.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xOrigDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xOrigDocSh), FORMAT_XLSX);
    xmlDocPtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);
    assertXPath(pSheet, "/x:worksheet/x:sheetFormatPr", "zeroHeight", "true" );
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row", 0);
}

void ScExportTest::testHiddenEmptyRowsXLSX()
{
    //tdf#98106 FILESAVE: Hidden and empty rows became visible when export to .XLSX
    ScDocShellRef xShell = loadDoc("hidden-empty-rows.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocPtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:sheetFormatPr",  "zeroHeight", "false" );
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "hidden", "false");
}

void ScExportTest::testLandscapeOrientationXLSX()
{
    //tdf#48767 - Landscape page orientation is not loaded from .xlsx format with MS Excel, after export with Libre Office
    ScDocShellRef xShell = loadDoc("hidden-empty-rows.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocPtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // the usePrinterDefaults cannot be saved to allow opening sheets in Landscape mode via MS Excel
    assertXPathNoAttribute(pSheet, "/x:worksheet/x:pageSetup", "usePrinterDefaults");
    assertXPath(pSheet, "/x:worksheet/x:pageSetup", "orientation", "landscape");
}

void ScExportTest::testDataBarExportXLSX()
{
    ScDocShellRef xShell = loadDoc("databar.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    testDataBar_Impl(rDoc);

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
        { "miscrowheights.", FORMAT_XLSX, FORMAT_XLSX, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        // Checks that some distributed ( non-empty ) heights remain set after export (to xls)
        { "miscrowheights.", FORMAT_XLSX, FORMAT_XLS, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        // Checks that repreated rows ( of various heights ) remain set after export ( to xlsx )
        { "miscemptyrepeatedrowheights.", FORMAT_ODS, FORMAT_XLSX, SAL_N_ELEMENTS(EmptyRepeatRowData), EmptyRepeatRowData },
        // Checks that repreated rows ( of various heights ) remain set after export ( to xls )
        { "miscemptyrepeatedrowheights.", FORMAT_ODS, FORMAT_XLS, SAL_N_ELEMENTS(EmptyRepeatRowData), EmptyRepeatRowData },
    };
    miscRowHeightsTest( aTestValues, SAL_N_ELEMENTS(aTestValues) );
}

namespace {

void setAttribute( ScFieldEditEngine& rEE, sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, sal_uInt16 nType, Color nColor = COL_BLACK )
{
    ESelection aSel;
    aSel.nStartPara = aSel.nEndPara = nPara;
    aSel.nStartPos = nStart;
    aSel.nEndPos = nEnd;

    SfxItemSet aItemSet = rEE.GetEmptyItemSet();
    switch (nType)
    {
        case EE_CHAR_WEIGHT:
        {
            SvxWeightItem aWeight(WEIGHT_BOLD, nType);
            aItemSet.Put(aWeight);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_ITALIC:
        {
            SvxPostureItem aItalic(ITALIC_NORMAL, nType);
            aItemSet.Put(aItalic);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_STRIKEOUT:
        {
            SvxCrossedOutItem aCrossOut(STRIKEOUT_SINGLE, nType);
            aItemSet.Put(aCrossOut);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_OVERLINE:
        {
            SvxOverlineItem aItem(LINESTYLE_DOUBLE, nType);
            aItemSet.Put(aItem);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_UNDERLINE:
        {
            SvxUnderlineItem aItem(LINESTYLE_DOUBLE, nType);
            aItemSet.Put(aItem);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        case EE_CHAR_COLOR:
        {
            SvxColorItem aItem(nColor, nType);
            aItemSet.Put(aItem);
            rEE.QuickSetAttribs(aItemSet, aSel);
        }
        break;
        default:
            ;
    }
}

void setFont( ScFieldEditEngine& rEE, sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, const OUString& rFontName )
{
    ESelection aSel;
    aSel.nStartPara = aSel.nEndPara = nPara;
    aSel.nStartPos = nStart;
    aSel.nEndPos = nEnd;

    SfxItemSet aItemSet = rEE.GetEmptyItemSet();
    SvxFontItem aItem(FAMILY_MODERN, rFontName, "", PITCH_VARIABLE, RTL_TEXTENCODING_UTF8, EE_CHAR_FONTINFO);
    aItemSet.Put(aItem);
    rEE.QuickSetAttribs(aItemSet, aSel);
}

void setEscapement( ScFieldEditEngine& rEE, sal_Int32 nPara, sal_Int32 nStart, sal_Int32 nEnd, short nEsc, sal_uInt8 nRelSize )
{
    ESelection aSel;
    aSel.nStartPara = aSel.nEndPara = nPara;
    aSel.nStartPos = nStart;
    aSel.nEndPos = nEnd;

    SfxItemSet aItemSet = rEE.GetEmptyItemSet();
    SvxEscapementItem aItem(nEsc, nRelSize, EE_CHAR_ESCAPEMENT);
    aItemSet.Put(aItem);
    rEE.QuickSetAttribs(aItemSet, aSel);
}

}

void ScExportTest::testNamedRangeBugfdo62729()
{
    ScDocShellRef xShell = loadDoc("fdo62729.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());
    ScDocument& rDoc = xShell->GetDocument();

    ScRangeName* pNames = rDoc.GetRangeName();
    //should be just a single named range
    CPPUNIT_ASSERT_EQUAL(size_t(1), pNames->size());
    rDoc.DeleteTab(0);
    //should be still a single named range
    CPPUNIT_ASSERT_EQUAL(size_t(1), pNames->size());
    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_ODS);
    xShell->DoClose();

    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc2 = xDocSh->GetDocument();

    pNames = rDoc2.GetRangeName();
    //after reload should still have a named range
    CPPUNIT_ASSERT_EQUAL(size_t(1), pNames->size());

    xDocSh->DoClose();
}

void ScExportTest::testBuiltinRangesXLSX()
{
    ScDocShellRef xShell = loadDoc("built-in_ranges.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());
    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xShell->DoClose();

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/workbook.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in names are still there
    assertXPathContent(pDoc, "/x:workbook/x:definedNames/x:definedName[@name='_xlnm._FilterDatabase'][@localSheetId='0']", "'Sheet1 Test'!$A$1:$A$5");
    assertXPathContent(pDoc, "/x:workbook/x:definedNames/x:definedName[@name='_xlnm._FilterDatabase'][@localSheetId='1']", "'Sheet2 Test'!$K$10:$K$14");
    assertXPathContent(pDoc, "/x:workbook/x:definedNames/x:definedName[@name='_xlnm.Print_Area'][@localSheetId='0']", "'Sheet1 Test'!$A$1:$A$5");
    assertXPathContent(pDoc, "/x:workbook/x:definedNames/x:definedName[@name='_xlnm.Print_Area'][@localSheetId='1']", "'Sheet2 Test'!$K$10:$M$18");

    //...and that no extra ones are added (see tdf#112571)
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName[@name='_xlnm._FilterDatabase_0'][@localSheetId='0']", 0);
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName[@name='_xlnm._FilterDatabase_0'][@localSheetId='1']", 0);
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName[@name='_xlnm.Print_Area_0'][@localSheetId='0']", 0);
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName[@name='_xlnm.Print_Area_0'][@localSheetId='1']", 0);

    xDocSh->DoClose();
}

void ScExportTest::testRichTextExportODS()
{
    struct
    {
        static bool isBold(const editeng::Section& rAttr)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_WEIGHT)
                    continue;

                return static_cast<const SvxWeightItem*>(p)->GetWeight() == WEIGHT_BOLD;
            }
            return false;
        }

        static bool isItalic(const editeng::Section& rAttr)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_ITALIC)
                    continue;

                return static_cast<const SvxPostureItem*>(p)->GetPosture() == ITALIC_NORMAL;
            }
            return false;
        }

        static bool isStrikeOut(const editeng::Section& rAttr)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_STRIKEOUT)
                    continue;

                return static_cast<const SvxCrossedOutItem*>(p)->GetStrikeout() == STRIKEOUT_SINGLE;
            }
            return false;
        }

        static bool isOverline(const editeng::Section& rAttr, FontLineStyle eStyle)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_OVERLINE)
                    continue;

                return static_cast<const SvxOverlineItem*>(p)->GetLineStyle() == eStyle;
            }
            return false;
        }

        static bool isUnderline(const editeng::Section& rAttr, FontLineStyle eStyle)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_UNDERLINE)
                    continue;

                return static_cast<const SvxUnderlineItem*>(p)->GetLineStyle() == eStyle;
            }
            return false;
        }

        static bool isFont(const editeng::Section& rAttr, const OUString& rFontName)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_FONTINFO)
                    continue;

                return static_cast<const SvxFontItem*>(p)->GetFamilyName() == rFontName;
            }
            return false;
        }

        static bool isEscapement(const editeng::Section& rAttr, short nEsc, sal_uInt8 nRelSize)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_ESCAPEMENT)
                    continue;

                const SvxEscapementItem* pItem = static_cast<const SvxEscapementItem*>(p);
                return ((pItem->GetEsc() == nEsc) && (pItem->GetProportionalHeight() == nRelSize));
            }
            return false;
        }

        static bool isColor(const editeng::Section& rAttr, Color nColor)
        {
            if (rAttr.maAttributes.empty())
                return false;

            std::vector<const SfxPoolItem*>::const_iterator it = rAttr.maAttributes.begin(), itEnd = rAttr.maAttributes.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                if (p->Which() != EE_CHAR_COLOR)
                    continue;

                return static_cast<const SvxColorItem*>(p)->GetValue() == nColor;
            }
            return false;
        }

        bool checkB2(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Bold and Italic")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 3)
                return false;

            // Check the first bold section.
            const editeng::Section* pAttr = &aSecAttrs[0];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 0 || pAttr->mnEnd != 4)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isBold(*pAttr))
                return false;

            // The middle section should be unformatted.
            pAttr = &aSecAttrs[1];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 4 || pAttr->mnEnd != 9)
                return false;

            if (!pAttr->maAttributes.empty())
                return false;

            // The last section should be italic.
            pAttr = &aSecAttrs[2];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 9 || pAttr->mnEnd != 15)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isItalic(*pAttr))
                return false;

            return true;
        }

        bool checkB4(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 3)
                return false;

            if (pText->GetText(0) != "One")
                return false;

            if (pText->GetText(1) != "Two")
                return false;

            if (pText->GetText(2) != "Three")
                return false;

            return true;
        }

        bool checkB5(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 6)
                return false;

            if (!pText->GetText(0).isEmpty())
                return false;

            if (pText->GetText(1) != "Two")
                return false;

            if (pText->GetText(2) != "Three")
                return false;

            if (!pText->GetText(3).isEmpty())
                return false;

            if (pText->GetText(4) != "Five")
                return false;

            if (!pText->GetText(5).isEmpty())
                return false;

            return true;
        }

        bool checkB6(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Strike Me")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 2)
                return false;

            // Check the first strike-out section.
            const editeng::Section* pAttr = &aSecAttrs[0];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 0 || pAttr->mnEnd != 6)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isStrikeOut(*pAttr))
                return false;

            // The last section should be unformatted.
            pAttr = &aSecAttrs[1];
            return pAttr->mnParagraph == 0 && pAttr->mnStart == 6 && pAttr->mnEnd == 9;
        }

        bool checkB7(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Font1 and Font2")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 3)
                return false;

            // First section should have "Courier" font applied.
            const editeng::Section* pAttr = &aSecAttrs[0];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 0 || pAttr->mnEnd != 5)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isFont(*pAttr, "Courier"))
                return false;

            // Last section should have "Luxi Mono" applied.
            pAttr = &aSecAttrs[2];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 10 || pAttr->mnEnd != 15)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isFont(*pAttr, "Luxi Mono"))
                return false;

            return true;
        }

        bool checkB8(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Over and Under")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 3)
                return false;

            // First section shoul have overline applied.
            const editeng::Section* pAttr = &aSecAttrs[0];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 0 || pAttr->mnEnd != 4)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isOverline(*pAttr, LINESTYLE_DOUBLE))
                return false;

            // Last section should have underline applied.
            pAttr = &aSecAttrs[2];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 9 || pAttr->mnEnd != 14)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isUnderline(*pAttr, LINESTYLE_DOUBLE))
                return false;

            return true;
        }

        bool checkB9(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "Sub and Super")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 3)
                return false;

            // superscript
            const editeng::Section* pAttr = &aSecAttrs[0];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 0 || pAttr->mnEnd != 3)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isEscapement(*pAttr, 32, 64))
                return false;

            // subscript
            pAttr = &aSecAttrs[2];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 8 || pAttr->mnEnd != 13)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isEscapement(*pAttr, -32, 66))
                return false;

            return true;
        }

        bool checkB10(const EditTextObject* pText) const
        {
            if (!pText)
                return false;

            if (pText->GetParagraphCount() != 1)
                return false;

            if (pText->GetText(0) != "BLUE AUTO")
                return false;

            std::vector<editeng::Section> aSecAttrs;
            pText->GetAllSections(aSecAttrs);
            if (aSecAttrs.size() != 2)
                return false;

            // auto color
            const editeng::Section* pAttr = &aSecAttrs[1];
            if (pAttr->mnParagraph != 0 ||pAttr->mnStart != 5 || pAttr->mnEnd != 9)
                return false;

            if (pAttr->maAttributes.size() != 1 || !isColor(*pAttr, COL_AUTO))
                return false;

            return true;
        }

    } aCheckFunc;

    // Start with an empty document, put one edit text cell, and make sure it
    // survives the save and reload.
    ScDocShellRef xOrigDocSh = loadDoc("empty.", FORMAT_ODS, true);
    const EditTextObject* pEditText;
    {
        ScDocument& rDoc = xOrigDocSh->GetDocument();
        CPPUNIT_ASSERT_MESSAGE("This document should at least have one sheet.", rDoc.GetTableCount() > 0);

        // Insert an edit text cell.
        ScFieldEditEngine* pEE = &rDoc.GetEditEngine();
        pEE->SetText("Bold and Italic");
        // Set the 'Bold' part bold.
        setAttribute(*pEE, 0, 0, 4, EE_CHAR_WEIGHT);
        // Set the 'Italic' part italic.
        setAttribute(*pEE, 0, 9, 15, EE_CHAR_ITALIC);
        ESelection aSel;
        aSel.nStartPara = aSel.nEndPara = 0;

        // Set this edit text to cell B2.
        rDoc.SetEditText(ScAddress(1,1,0), pEE->CreateTextObject());
        pEditText = rDoc.GetEditText(ScAddress(1,1,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B2 value.", aCheckFunc.checkB2(pEditText));
    }

    // Now, save and reload this document.
    ScDocShellRef xNewDocSh = saveAndReload(xOrigDocSh.get(), FORMAT_ODS);
    {
        xOrigDocSh->DoClose();
        CPPUNIT_ASSERT(xNewDocSh.is());
        ScDocument& rDoc2 = xNewDocSh->GetDocument();
        CPPUNIT_ASSERT_MESSAGE("Reloaded document should at least have one sheet.", rDoc2.GetTableCount() > 0);
        ScFieldEditEngine* pEE = &rDoc2.GetEditEngine();

        // Make sure the content of B2 is still intact.
        CPPUNIT_ASSERT_MESSAGE("Incorrect B2 value.", aCheckFunc.checkB2(pEditText));

        // Insert a multi-line content to B4.
        pEE->Clear();
        pEE->SetText("One\nTwo\nThree");
        rDoc2.SetEditText(ScAddress(1,3,0), pEE->CreateTextObject());
        pEditText = rDoc2.GetEditText(ScAddress(1,3,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B4 value.", aCheckFunc.checkB4(pEditText));
    }

    // Reload the doc again, and check the content of B2 and B4.
    ScDocShellRef xNewDocSh2 = saveAndReload(xNewDocSh.get(), FORMAT_ODS);
    {
        ScDocument& rDoc3 = xNewDocSh2->GetDocument();
        ScFieldEditEngine* pEE = &rDoc3.GetEditEngine();
        xNewDocSh->DoClose();

        pEditText = rDoc3.GetEditText(ScAddress(1,1,0));
        CPPUNIT_ASSERT_MESSAGE("B2 should be an edit text.", pEditText);
        pEditText = rDoc3.GetEditText(ScAddress(1,3,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B4 value.", aCheckFunc.checkB4(pEditText));

        // Insert a multi-line content to B5, but this time, set some empty paragraphs.
        pEE->Clear();
        pEE->SetText("\nTwo\nThree\n\nFive\n");
        rDoc3.SetEditText(ScAddress(1,4,0), pEE->CreateTextObject());
        pEditText = rDoc3.GetEditText(ScAddress(1,4,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B5 value.", aCheckFunc.checkB5(pEditText));

        // Insert a text with strikethrough in B6.
        pEE->Clear();
        pEE->SetText("Strike Me");
        // Set the 'Strike' part strikethrough.
        setAttribute(*pEE, 0, 0, 6, EE_CHAR_STRIKEOUT);
        rDoc3.SetEditText(ScAddress(1,5,0), pEE->CreateTextObject());
        pEditText = rDoc3.GetEditText(ScAddress(1,5,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B6 value.", aCheckFunc.checkB6(pEditText));

        // Insert a text with different font segments in B7.
        pEE->Clear();
        pEE->SetText("Font1 and Font2");
        setFont(*pEE, 0, 0, 5, "Courier");
        setFont(*pEE, 0, 10, 15, "Luxi Mono");
        rDoc3.SetEditText(ScAddress(1,6,0), pEE->CreateTextObject());
        pEditText = rDoc3.GetEditText(ScAddress(1,6,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B7 value.", aCheckFunc.checkB7(pEditText));

        // Insert a text with overline and underline in B8.
        pEE->Clear();
        pEE->SetText("Over and Under");
        setAttribute(*pEE, 0, 0, 4, EE_CHAR_OVERLINE);
        setAttribute(*pEE, 0, 9, 14, EE_CHAR_UNDERLINE);
        rDoc3.SetEditText(ScAddress(1,7,0), pEE->CreateTextObject());
        pEditText = rDoc3.GetEditText(ScAddress(1,7,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B8 value.", aCheckFunc.checkB8(pEditText));

        pEE->Clear();
        pEE->SetText("Sub and Super");
        setEscapement(*pEE, 0, 0, 3, 32, 64);
        setEscapement(*pEE, 0, 8, 13, -32, 66);
        rDoc3.SetEditText(ScAddress(1,8,0), pEE->CreateTextObject());
        pEditText = rDoc3.GetEditText(ScAddress(1,8,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B9 value.", aCheckFunc.checkB9(pEditText));

        ScPatternAttr aCellFontColor(rDoc3.GetPool());
        aCellFontColor.GetItemSet().Put(SvxColorItem(COL_BLUE, ATTR_FONT_COLOR));
        // Set font color of B10 to blue.
        rDoc3.ApplyPattern(1, 9, 0, aCellFontColor);
        pEE->Clear();
        pEE->SetText("BLUE AUTO");
        // Set the color of the string "AUTO" to automatic color.
        setAttribute(*pEE, 0, 5, 9, EE_CHAR_COLOR, COL_AUTO);
        rDoc3.SetEditText(ScAddress(1, 9, 0), pEE->CreateTextObject());
        pEditText = rDoc3.GetEditText(ScAddress(1, 9, 0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B10 value.", aCheckFunc.checkB10(pEditText));
    }

    // Reload the doc again, and check the content of B2, B4, B6 and B7.
    ScDocShellRef xNewDocSh3 = saveAndReload(xNewDocSh2.get(), FORMAT_ODS);
    ScDocument& rDoc4 = xNewDocSh3->GetDocument();
    xNewDocSh2->DoClose();

    pEditText = rDoc4.GetEditText(ScAddress(1,1,0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B2 value after save and reload.", aCheckFunc.checkB2(pEditText));
    pEditText = rDoc4.GetEditText(ScAddress(1,3,0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B4 value after save and reload.", aCheckFunc.checkB4(pEditText));
    pEditText = rDoc4.GetEditText(ScAddress(1,4,0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B5 value after save and reload.", aCheckFunc.checkB5(pEditText));
    pEditText = rDoc4.GetEditText(ScAddress(1,5,0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B6 value after save and reload.", aCheckFunc.checkB6(pEditText));
    pEditText = rDoc4.GetEditText(ScAddress(1,6,0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B7 value after save and reload.", aCheckFunc.checkB7(pEditText));
    pEditText = rDoc4.GetEditText(ScAddress(1,7,0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B8 value after save and reload.", aCheckFunc.checkB8(pEditText));
    pEditText = rDoc4.GetEditText(ScAddress(1,9,0));
    CPPUNIT_ASSERT_MESSAGE("Incorrect B10 value after save and reload.", aCheckFunc.checkB10(pEditText));

    xNewDocSh3->DoClose();
}

void ScExportTest::testRichTextCellFormatXLSX()
{
    ScDocShellRef xDocSh = loadDoc("cellformat.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocPtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // make sure the only cell in this doc is assigned some formatting record
    OUString aCellFormat = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row/x:c", "s");
    CPPUNIT_ASSERT_MESSAGE("Cell format is missing", !aCellFormat.isEmpty());

    xmlDocPtr pStyles = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/styles.xml");
    CPPUNIT_ASSERT(pStyles);

    OString nFormatIdx = OString::number( aCellFormat.toInt32() + 1 );
    const OString aXPath1( "/x:styleSheet/x:cellXfs/x:xf[" + nFormatIdx + "]/x:alignment" );
    // formatting record is set to wrap text
    assertXPath(pStyles, aXPath1, "wrapText", "true");

    // see what font it references
    const OString aXPath2( "/x:styleSheet/x:cellXfs/x:xf[" + nFormatIdx +"]" );
    OUString aFontId = getXPath(pStyles, aXPath2, "fontId");
    OString nFontIdx = OString::number( aFontId.toInt32() + 1 );

    // that font should be bold
    const OString aXPath3("/x:styleSheet/x:fonts/x:font[" + nFontIdx + "]/x:b");
    assertXPath(pStyles, aXPath3, "val", "true");

    xDocSh->DoClose();
}

void ScExportTest::testFormulaRefSheetNameODS()
{
    ScDocShellRef xDocSh = loadDoc("formula-quote-in-sheet-name.", FORMAT_ODS, true);
    {
        ScDocument& rDoc = xDocSh->GetDocument();

        sc::AutoCalcSwitch aACSwitch(rDoc, true); // turn on auto calc.
        rDoc.SetString(ScAddress(1,1,0), "='90''s Data'.B2");
        CPPUNIT_ASSERT_EQUAL(1.1, rDoc.GetValue(ScAddress(1,1,0)));
        ASSERT_FORMULA_EQUAL(rDoc, ScAddress(1,1,0), "'90''s Data'.B2", "Wrong formula");
    }
    // Now, save and reload this document.
    ScDocShellRef xNewDocSh = saveAndReload(xDocSh.get(), FORMAT_ODS);
    xDocSh->DoClose();

    ScDocument& rDoc = xNewDocSh->GetDocument();
    rDoc.CalcAll();
    CPPUNIT_ASSERT_EQUAL(1.1, rDoc.GetValue(ScAddress(1,1,0)));
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(1,1,0), "'90''s Data'.B2", "Wrong formula");

    xNewDocSh->DoClose();
}

void ScExportTest::testCellValuesExportODS()
{
    // Start with an empty document
    ScDocShellRef xOrigDocSh = loadDoc("empty.", FORMAT_ODS);
    {
        ScDocument& rDoc = xOrigDocSh->GetDocument();
        CPPUNIT_ASSERT_MESSAGE("This document should at least have one sheet.", rDoc.GetTableCount() > 0);

        // set a value double
        rDoc.SetValue(ScAddress(0,0,0), 2.0); // A1

        // set a formula
        rDoc.SetValue(ScAddress(2,0,0), 3.0); // C1
        rDoc.SetValue(ScAddress(3,0,0), 3); // D1
        rDoc.SetString(ScAddress(4,0,0), "=10*C1/4"); // E1
        rDoc.SetValue(ScAddress(5,0,0), 3.0); // F1
        rDoc.SetString(ScAddress(7,0,0), "=SUM(C1:F1)"); //H1

        // set a string
        rDoc.SetString(ScAddress(0,2,0), "a simple line"); //A3

        // set a digit string
        rDoc.SetString(ScAddress(0,4,0), "'12"); //A5
        // set a contiguous value
        rDoc.SetValue(ScAddress(0,5,0), 12.0); //A6
        // set a contiguous string
        rDoc.SetString(ScAddress(0,6,0), "a string"); //A7
        // set a contiguous formula
        rDoc.SetString(ScAddress(0,7,0), "=$A$6"); //A8
    }
    // save and reload
    ScDocShellRef xNewDocSh = saveAndReload(xOrigDocSh.get(), FORMAT_ODS);
    xOrigDocSh->DoClose();
    CPPUNIT_ASSERT(xNewDocSh.is());
    ScDocument& rDoc = xNewDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("Reloaded document should at least have one sheet.", rDoc.GetTableCount() > 0);

    // check value
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(0,0,0));
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(2,0,0));
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(3,0,0));
    CPPUNIT_ASSERT_EQUAL(7.5, rDoc.GetValue(4,0,0));
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(5,0,0));

    // check formula
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(4,0,0), "10*C1/4", "Wrong formula =10*C1/4");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(7,0,0), "SUM(C1:F1)", "Wrong formula =SUM(C1:F1)");
    CPPUNIT_ASSERT_EQUAL(16.5, rDoc.GetValue(7,0,0));

    // check string
    ScRefCellValue aCell;
    aCell.assign(rDoc, ScAddress(0,2,0));
    CPPUNIT_ASSERT_EQUAL( CELLTYPE_STRING, aCell.meType );

    // check for an empty cell
    aCell.assign(rDoc, ScAddress(0,3,0));
    CPPUNIT_ASSERT_EQUAL( CELLTYPE_NONE, aCell.meType);

    // check a digit string
    aCell.assign(rDoc, ScAddress(0,4,0));
    CPPUNIT_ASSERT_EQUAL( CELLTYPE_STRING, aCell.meType);

    //check contiguous values
    CPPUNIT_ASSERT_EQUAL( 12.0, rDoc.GetValue(0,5,0) );
    CPPUNIT_ASSERT_EQUAL( OUString("a string"), rDoc.GetString(0,6,0) );
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(0,7,0), "$A$6", "Wrong formula =$A$6");
    CPPUNIT_ASSERT_EQUAL( rDoc.GetValue(0,5,0), rDoc.GetValue(0,7,0) );

    xNewDocSh->DoClose();
}

void ScExportTest::testCellNoteExportODS()
{
    ScDocShellRef xOrigDocSh = loadDoc("single-note.", FORMAT_ODS);
    ScAddress aPos(0,0,0); // Start with A1.
    {
        ScDocument& rDoc = xOrigDocSh->GetDocument();

        CPPUNIT_ASSERT_MESSAGE("There should be a note at A1.", rDoc.HasNote(aPos));

        aPos.IncRow(); // Move to A2.
        ScPostIt* pNote = rDoc.GetOrCreateNote(aPos);
        pNote->SetText(aPos, "Note One");
        pNote->SetAuthor("Author One");
        CPPUNIT_ASSERT_MESSAGE("There should be a note at A2.", rDoc.HasNote(aPos));
    }
    // save and reload
    ScDocShellRef xNewDocSh = saveAndReload(xOrigDocSh.get(), FORMAT_ODS);
    xOrigDocSh->DoClose();
    CPPUNIT_ASSERT(xNewDocSh.is());
    ScDocument& rDoc = xNewDocSh->GetDocument();

    aPos.SetRow(0); // Move back to A1.
    CPPUNIT_ASSERT_MESSAGE("There should be a note at A1.", rDoc.HasNote(aPos));
    aPos.IncRow(); // Move to A2.
    CPPUNIT_ASSERT_MESSAGE("There should be a note at A2.", rDoc.HasNote(aPos));

    xNewDocSh->DoClose();
}

void ScExportTest::testCellNoteExportXLS()
{
    // Start with an empty document.s
    ScDocShellRef xOrigDocSh = loadDoc("notes-on-3-sheets.", FORMAT_ODS);
    {
        ScDocument& rDoc = xOrigDocSh->GetDocument();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("This document should have 3 sheets.", SCTAB(3), rDoc.GetTableCount());

        // Check note's presence.
        CPPUNIT_ASSERT( rDoc.HasNote(ScAddress(0,0,0)));
        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,1,0)));
        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,2,0)));

        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,0,1)));
        CPPUNIT_ASSERT( rDoc.HasNote(ScAddress(0,1,1)));
        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,2,1)));

        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,0,2)));
        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,1,2)));
        CPPUNIT_ASSERT( rDoc.HasNote(ScAddress(0,2,2)));
    }
    // save and reload as XLS.
    ScDocShellRef xNewDocSh = saveAndReload(xOrigDocSh.get(), FORMAT_XLS);
    {
        xOrigDocSh->DoClose();
        CPPUNIT_ASSERT(xNewDocSh.is());
        ScDocument& rDoc = xNewDocSh->GetDocument();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("This document should have 3 sheets.", SCTAB(3), rDoc.GetTableCount());

        // Check note's presence again.
        CPPUNIT_ASSERT( rDoc.HasNote(ScAddress(0,0,0)));
        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,1,0)));
        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,2,0)));

        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,0,1)));
        CPPUNIT_ASSERT( rDoc.HasNote(ScAddress(0,1,1)));
        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,2,1)));

        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,0,2)));
        CPPUNIT_ASSERT(!rDoc.HasNote(ScAddress(0,1,2)));
        CPPUNIT_ASSERT( rDoc.HasNote(ScAddress(0,2,2)));

        xNewDocSh->DoClose();
    }
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
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong matrix range.", rRange, aMatRange);
            const ScFormulaCell* pCell = rDoc.GetFormulaCell(aPos);
            CPPUNIT_ASSERT_MESSAGE("This must be a formula cell.", pCell);

            bIsMatrix = pCell->GetMatrixOrigin(aMatOrigin);
            CPPUNIT_ASSERT_MESSAGE("Not a part of matrix formula.", bIsMatrix);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong matrix origin.", aMatRange.aStart, aMatOrigin);
        }
    }
}

}

void ScExportTest::testInlineArrayXLS()
{
    ScDocShellRef xShell = loadDoc("inline-array.", FORMAT_XLS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // B2:C3 contains a matrix.
    checkMatrixRange(rDoc, ScRange(1,1,0,2,2,0));

    // B5:D6 contains a matrix.
    checkMatrixRange(rDoc, ScRange(1,4,0,3,5,0));

    // B8:C10 as well.
    checkMatrixRange(rDoc, ScRange(1,7,0,2,9,0));

    xDocSh->DoClose();
}

void ScExportTest::testEmbeddedChartODS()
{
    ScDocShellRef xShell = loadDoc("embedded-chart.", FORMAT_XLS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pTempFile(
            ScBootstrapFixture::exportTo(xShell.get(), FORMAT_ODS));

    xmlDocPtr pDoc = XPathHelper::parseExport(pTempFile, m_xSFactory, "content.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc,
        "/office:document-content/office:body/office:spreadsheet/table:table[2]/table:table-row[7]/table:table-cell[2]/draw:frame/draw:object",
        "notify-on-update-of-ranges",
        "Chart1.B3:Chart1.B5 Chart1.C2:Chart1.C2 Chart1.C3:Chart1.C5");

    xShell->DoClose();
}

void ScExportTest::testEmbeddedChartXLS()
{
    ScDocShellRef xShell = loadDoc("embedded-chart.", FORMAT_XLS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Make sure the 2nd sheet is named 'Chart1'.
    OUString aName;
    rDoc.GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Chart1"), aName);

    const SdrOle2Obj* pOleObj = getSingleChartObject(rDoc, 1);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    ScRangeList aRanges = getChartRanges(rDoc, *pOleObj);
    CPPUNIT_ASSERT_MESSAGE("Label range (B3:B5) not found.", aRanges.In(ScRange(1,2,1,1,4,1)));
    CPPUNIT_ASSERT_MESSAGE("Data label (C2) not found.", aRanges.In(ScAddress(2,1,1)));
    CPPUNIT_ASSERT_MESSAGE("Data range (C3:C5) not found.", aRanges.In(ScRange(2,2,1,2,4,1)));

    xDocSh->DoClose();
}

void ScExportTest::testCellAnchoredGroupXLS()
{
    ScDocShellRef xDocSh_in = loadDoc("cell-anchored-group.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-anchored-group.xls", xDocSh_in.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xDocSh_in), FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to save and reload cell-anchored-group.ods", xDocSh.is());

    // the document contains a group anchored on the first cell, make sure it's there in the right place
    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", rDoc.GetTableCount() > 0);
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be 1 objects.", static_cast<size_t>(1), nCount);

    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Upper left of bounding rectangle should be nonnegative.",
        pData->getShapeRect().Left() >= 0 || pData->getShapeRect().Top() >= 0);
    xDocSh->DoClose();
}

void ScExportTest::testFormulaReferenceXLS()
{
    ScDocShellRef xShell = loadDoc("formula-reference.", FORMAT_XLS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,1,0), "$A$2+$B$2+$C$2", "Wrong formula in D2");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,2,0), "A3+B3+C3", "Wrong formula in D3");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,5,0), "SUM($A$6:$C$6)", "Wrong formula in D6");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,6,0), "SUM(A7:C7)", "Wrong formula in D7");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,9,0), "$Two.$A$2+$Two.$B$2+$Two.$C$2", "Wrong formula in D10");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,10,0), "$Two.A3+$Two.B3+$Two.C3", "Wrong formula in D11");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,13,0), "MIN($Two.$A$2:$C$2)", "Wrong formula in D14");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,14,0), "MAX($Two.A3:C3)", "Wrong formula in D15");

    xDocSh->DoClose();
}

void ScExportTest::testSheetProtectionXLSX()
{
    ScDocShellRef xShell = loadDoc("ProtecteSheet1234Pass.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const ScTableProtection* pTabProtect = rDoc.GetTabProtection(0);
    CPPUNIT_ASSERT(pTabProtect);
    Sequence<sal_Int8> aHash = pTabProtect->getPasswordHash(PASSHASH_XL);
    // check has
    if (aHash.getLength() >= 2)
    {
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(204), static_cast<sal_uInt8>(aHash[0]));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(61), static_cast<sal_uInt8>(aHash[1]));
    }
    // we could flesh out this check I guess
    CPPUNIT_ASSERT ( !pTabProtect->isOptionEnabled( ScTableProtection::OBJECTS ) );
    CPPUNIT_ASSERT ( !pTabProtect->isOptionEnabled( ScTableProtection::SCENARIOS ) );
    xDocSh->DoClose();
}

void ScExportTest::testSheetProtectionXLSB()
{
    ScDocShellRef xShell = loadDoc("tdf108017_calcProtection.", FORMAT_XLSB);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const ScTableProtection* pTabProtect = rDoc.GetTabProtection(0);
    CPPUNIT_ASSERT(pTabProtect);
    CPPUNIT_ASSERT(pTabProtect->isOptionEnabled( ScTableProtection::SELECT_UNLOCKED_CELLS ));
    CPPUNIT_ASSERT(!pTabProtect->isOptionEnabled( ScTableProtection::SELECT_LOCKED_CELLS ));
    xDocSh->DoClose();
}

namespace {

const char* toBorderName( SvxBorderLineStyle eStyle )
{
    switch (eStyle)
    {
        case SvxBorderLineStyle::SOLID: return "SOLID";
        case SvxBorderLineStyle::DOTTED: return "DOTTED";
        case SvxBorderLineStyle::DASHED: return "DASHED";
        case SvxBorderLineStyle::DASH_DOT: return "DASH_DOT";
        case SvxBorderLineStyle::DASH_DOT_DOT: return "DASH_DOT_DOT";
        case SvxBorderLineStyle::DOUBLE_THIN: return "DOUBLE_THIN";
        case SvxBorderLineStyle::FINE_DASHED: return "FINE_DASHED";
        default:
            ;
    }

    return "";
}

}

void ScExportTest::testExcelCellBorders( sal_uLong nFormatType )
{
    struct
    {
        SCROW mnRow;
        SvxBorderLineStyle mnStyle;
        long mnWidth;
    } aChecks[] = {
        {  1, SvxBorderLineStyle::SOLID,         1 }, // hair
        {  3, SvxBorderLineStyle::DOTTED,       15 }, // dotted
        {  5, SvxBorderLineStyle::DASH_DOT_DOT, 15 }, // dash dot dot
        {  7, SvxBorderLineStyle::DASH_DOT,     15 }, // dash dot
        {  9, SvxBorderLineStyle::FINE_DASHED,  15 }, // dashed
        { 11, SvxBorderLineStyle::SOLID,        15 }, // thin
        { 13, SvxBorderLineStyle::DASH_DOT_DOT, 35 }, // medium dash dot dot
        { 17, SvxBorderLineStyle::DASH_DOT,     35 }, // medium dash dot
        { 19, SvxBorderLineStyle::DASHED,       35 }, // medium dashed
        { 21, SvxBorderLineStyle::SOLID,        35 }, // medium
        { 23, SvxBorderLineStyle::SOLID,        50 }, // thick
        { 25, SvxBorderLineStyle::DOUBLE_THIN,  -1 }, // double (don't check width)
    };

    ScDocShellRef xDocSh = loadDoc("cell-borders.", nFormatType);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file", xDocSh.is());
    {
        ScDocument& rDoc = xDocSh->GetDocument();

        for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
        {
            const editeng::SvxBorderLine* pLine = nullptr;
            rDoc.GetBorderLines(2, aChecks[i].mnRow, 0, nullptr, &pLine, nullptr, nullptr);
            CPPUNIT_ASSERT(pLine);
            CPPUNIT_ASSERT_EQUAL(toBorderName(aChecks[i].mnStyle), toBorderName(pLine->GetBorderLineStyle()));
            if (aChecks[i].mnWidth >= 0)
                CPPUNIT_ASSERT_EQUAL(aChecks[i].mnWidth, pLine->GetWidth());
        }
    }

    ScDocShellRef xNewDocSh = saveAndReload(xDocSh.get(), nFormatType);
    xDocSh->DoClose();
    ScDocument& rDoc = xNewDocSh->GetDocument();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
    {
        const editeng::SvxBorderLine* pLine = nullptr;
        rDoc.GetBorderLines(2, aChecks[i].mnRow, 0, nullptr, &pLine, nullptr, nullptr);
        CPPUNIT_ASSERT(pLine);
        CPPUNIT_ASSERT_EQUAL(toBorderName(aChecks[i].mnStyle), toBorderName(pLine->GetBorderLineStyle()));
        if (aChecks[i].mnWidth >= 0)
            CPPUNIT_ASSERT_EQUAL(aChecks[i].mnWidth, pLine->GetWidth());
    }

    xNewDocSh->DoClose();
}

void ScExportTest::testCellBordersXLS()
{
    testExcelCellBorders(FORMAT_XLS);
}

void ScExportTest::testCellBordersXLSX()
{
    testExcelCellBorders(FORMAT_XLSX);
}

void ScExportTest::testBordersExchangeXLSX()
{
    // Document: sc/qa/unit/data/README.cellborders

    // short name for the table
    const SvxBorderLineStyle None     = SvxBorderLineStyle::NONE;
    const SvxBorderLineStyle Solid    = SvxBorderLineStyle::SOLID;
    const SvxBorderLineStyle Dotted   = SvxBorderLineStyle::DOTTED;
    const SvxBorderLineStyle Dashed   = SvxBorderLineStyle::DASHED;
    const SvxBorderLineStyle FineDash = SvxBorderLineStyle::FINE_DASHED;
    const SvxBorderLineStyle DashDot  = SvxBorderLineStyle::DASH_DOT;
    const SvxBorderLineStyle DashDoDo = SvxBorderLineStyle::DASH_DOT_DOT;
    const SvxBorderLineStyle DoubThin = SvxBorderLineStyle::DOUBLE_THIN;

    const size_t nMaxCol = 18;
    const size_t nMaxRow = 7;

    static struct
    {
        SvxBorderLineStyle BorderStyleTop, BorderStyleBottom;
        long                      WidthTop, WidthBottom;
    } aCheckBorderWidth[nMaxCol][nMaxRow] =
    {
/*  Line               1                                2                              3                            4                             5                                6                              7
                     SOLID                           DOTTED                          DASHED                     FINE_DASHED                    DASH_DOT                      DASH_DOT_DOT                  DOUBLE_THIN          */
/*Width */
/* 0,05 */   {{Solid   , Solid   ,  1,  1}, {Dotted  , Dotted  , 15, 15}, {Dotted  , Dotted  , 15, 15}, {FineDash, FineDash, 15, 15}, {FineDash, FineDash, 15, 15}, {FineDash, FineDash, 15, 15}, {None    , None    ,  0,  0}},
/* 0,25 */   {{Solid   , Solid   ,  1,  1}, {Dotted  , Dotted  , 15, 15}, {Dotted  , Dotted  , 15, 15}, {FineDash, FineDash, 15, 15}, {FineDash, FineDash, 15, 15}, {FineDash, FineDash, 15, 15}, {None    , None    ,  0,  0}},
/* 0,50 */   {{Solid   , Solid   ,  1,  1}, {Dotted  , Dotted  , 15, 15}, {Dotted  , Dotted  , 15, 15}, {FineDash, FineDash, 15, 15}, {FineDash, FineDash, 15, 15}, {FineDash, FineDash, 15, 15}, {None    , None    ,  0,  0}},
/* 0,75 */   {{Solid   , Solid   , 15, 15}, {Dotted  , Dotted  , 15, 15}, {FineDash, FineDash, 15, 15}, {FineDash, FineDash, 15, 15}, {DashDot , DashDot , 15, 15}, {DashDoDo, DashDoDo, 15, 15}, {DoubThin, DoubThin, 35, 35}},
/* 1,00 */   {{Solid   , Solid   , 15, 15}, {Dotted  , Dotted  , 15, 15}, {FineDash, FineDash, 15, 15}, {FineDash, FineDash, 15, 15}, {DashDot , DashDot , 15, 15}, {DashDoDo, DashDoDo, 15, 15}, {DoubThin, DoubThin, 35, 35}},
/* 1,25 */   {{Solid   , Solid   , 15, 15}, {Dotted  , Dotted  , 15, 15}, {FineDash, FineDash, 15, 15}, {FineDash, FineDash, 15, 15}, {DashDot , DashDot , 15, 15}, {DashDoDo, DashDoDo, 15, 15}, {DoubThin, DoubThin, 35, 35}},
/* 1,50 */   {{Solid   , Solid   , 15, 15}, {Dotted  , Dotted  , 15, 15}, {FineDash, FineDash, 15, 15}, {FineDash, FineDash, 15, 15}, {DashDot , DashDot , 15, 15}, {DashDoDo, DashDoDo, 15, 15}, {DoubThin, DoubThin, 35, 35}},

/* 1,75 */   {{Solid   , Solid   , 35, 35}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}},
/* 2,00 */   {{Solid   , Solid   , 35, 35}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}},
/* 2,25 */   {{Solid   , Solid   , 35, 35}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}},

/* 2,50 */   {{Solid   , Solid   , 50, 50}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}},
/* 2,75 */   {{Solid   , Solid   , 50, 50}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}},
/* 3,00 */   {{Solid   , Solid   , 50, 50}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}},
/* 3,50 */   {{Solid   , Solid   , 50, 50}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}},
/* 4,00 */   {{Solid   , Solid   , 50, 50}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}},
/* 5,00 */   {{Solid   , Solid   , 50, 50}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}},
/* 7,00 */   {{Solid   , Solid   , 50, 50}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}},
/* 9,00 */   {{Solid   , Solid   , 50, 50}, {FineDash, FineDash, 35, 35}, {Dashed  , Dashed  , 35, 35}, {FineDash, FineDash, 35, 35}, {DashDot , DashDot , 35, 35}, {DashDoDo, DashDoDo, 35, 35}, {DoubThin, DoubThin, 35, 35}}
    };

    ScDocShellRef xShell    = loadDoc("test_borders_export.", FORMAT_ODS);  // load the ods with our Borders
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);          // save the ods to xlsx and load xlsx
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc    = xDocSh->GetDocument();

    for (size_t nCol = 0; nCol < nMaxCol; ++nCol)
    {
        for (size_t nRow = 0; nRow < nMaxRow; ++nRow)
        {
            const editeng::SvxBorderLine* pLineTop    = nullptr;
            const editeng::SvxBorderLine* pLineBottom = nullptr;
            rDoc.GetBorderLines(nCol + 2, (nRow * 2) + 8, 0, nullptr, &pLineTop, nullptr, &pLineBottom);
            if((nCol < 3) && (nRow == 6))
            {   // in this range no lines since minimum size to create a double is 0.5
                CPPUNIT_ASSERT(!pLineTop);
                CPPUNIT_ASSERT(!pLineBottom);
                continue;
            }
            else
            {
                CPPUNIT_ASSERT(pLineTop);
                CPPUNIT_ASSERT(pLineBottom);
            }

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Top Border-Line-Style wrong", aCheckBorderWidth[nCol][nRow].BorderStyleTop,
                                          pLineTop->GetBorderLineStyle());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom Border-Line-Style wrong", aCheckBorderWidth[nCol][nRow].BorderStyleBottom,
                                          pLineBottom->GetBorderLineStyle());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Top Width-Line wrong", aCheckBorderWidth[nCol][nRow].WidthTop,
                                          pLineTop->GetWidth());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom Width-Line wrong", aCheckBorderWidth[nCol][nRow].WidthBottom,
                                          pLineBottom->GetWidth());
        }
    }

    xDocSh->DoClose();
}

OUString toString( const ScBigRange& rRange )
{
    OUStringBuffer aBuf;
    aBuf.append("(columns:");
    aBuf.append(rRange.aStart.Col());
    aBuf.append('-');
    aBuf.append(rRange.aEnd.Col());
    aBuf.append(";rows:");
    aBuf.append(rRange.aStart.Row());
    aBuf.append('-');
    aBuf.append(rRange.aEnd.Row());
    aBuf.append(";sheets:");
    aBuf.append(rRange.aStart.Tab());
    aBuf.append('-');
    aBuf.append(rRange.aEnd.Tab());
    aBuf.append(')');

    return aBuf.makeStringAndClear();
}

void ScExportTest::testTrackChangesSimpleXLSX()
{
    struct CheckItem
    {
        sal_uLong mnActionId;
        ScChangeActionType meType;

        sal_Int32 mnStartCol;
        sal_Int32 mnStartRow;
        sal_Int32 mnStartTab;
        sal_Int32 mnEndCol;
        sal_Int32 mnEndRow;
        sal_Int32 mnEndTab;

        bool mbRowInsertedAtBottom;
    };

    struct
    {
        bool checkRange( ScChangeActionType eType, const ScBigRange& rExpected, const ScBigRange& rActual )
        {
            ScBigRange aExpected(rExpected), aActual(rActual);

            switch (eType)
            {
                case SC_CAT_INSERT_ROWS:
                {
                    // Ignore columns.
                    aExpected.aStart.SetCol(0);
                    aExpected.aEnd.SetCol(0);
                    aActual.aStart.SetCol(0);
                    aActual.aEnd.SetCol(0);
                }
                break;
                default:
                    ;
            }

            return aExpected == aActual;
        }

        bool check( const ScDocument& rDoc )
        {
            CheckItem aChecks[] =
            {
                {  1, SC_CAT_CONTENT     , 1, 1, 0, 1, 1, 0, false },
                {  2, SC_CAT_INSERT_ROWS , 0, 2, 0, 0, 2, 0, true },
                {  3, SC_CAT_CONTENT     , 1, 2, 0, 1, 2, 0, false },
                {  4, SC_CAT_INSERT_ROWS , 0, 3, 0, 0, 3, 0, true  },
                {  5, SC_CAT_CONTENT     , 1, 3, 0, 1, 3, 0, false },
                {  6, SC_CAT_INSERT_ROWS , 0, 4, 0, 0, 4, 0, true  },
                {  7, SC_CAT_CONTENT     , 1, 4, 0, 1, 4, 0, false },
                {  8, SC_CAT_INSERT_ROWS , 0, 5, 0, 0, 5, 0, true  },
                {  9, SC_CAT_CONTENT     , 1, 5, 0, 1, 5, 0, false },
                { 10, SC_CAT_INSERT_ROWS , 0, 6, 0, 0, 6, 0, true  },
                { 11, SC_CAT_CONTENT     , 1, 6, 0, 1, 6, 0, false },
                { 12, SC_CAT_INSERT_ROWS , 0, 7, 0, 0, 7, 0, true  },
                { 13, SC_CAT_CONTENT     , 1, 7, 0, 1, 7, 0, false },
            };

            ScChangeTrack* pCT = rDoc.GetChangeTrack();
            if (!pCT)
            {
                cerr << "Change track instance doesn't exist." << endl;
                return false;
            }

            sal_uLong nActionMax = pCT->GetActionMax();
            if (nActionMax != 13)
            {
                cerr << "Unexpected highest action ID value." << endl;
                return false;
            }

            for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
            {
                sal_uInt16 nActId = aChecks[i].mnActionId;
                const ScChangeAction* pAction = pCT->GetAction(nActId);
                if (!pAction)
                {
                    cerr << "No action for action number " << nActId << " found." << endl;
                    return false;
                }

                if (pAction->GetType() != aChecks[i].meType)
                {
                    cerr << "Unexpected action type for action number " << nActId << "." << endl;
                    return false;
                }

                const ScBigRange& rRange = pAction->GetBigRange();
                ScBigRange aCheck(aChecks[i].mnStartCol, aChecks[i].mnStartRow, aChecks[i].mnStartTab,
                                  aChecks[i].mnEndCol, aChecks[i].mnEndRow, aChecks[i].mnEndTab);

                if (!checkRange(pAction->GetType(), aCheck, rRange))
                {
                    cerr << "Unexpected range for action number " << nActId
                        << ": expected=" << toString(aCheck) << " actual=" << toString(rRange) << endl;
                    return false;
                }

                switch (pAction->GetType())
                {
                    case SC_CAT_INSERT_ROWS:
                    {
                        const ScChangeActionIns* p = static_cast<const ScChangeActionIns*>(pAction);
                        if (p->IsEndOfList() != aChecks[i].mbRowInsertedAtBottom)
                        {
                            cerr << "Unexpected end-of-list flag for action number " << nActId << "." << endl;
                            return false;
                        }
                    }
                    break;
                    default:
                        ;
                }
            }

            return true;
        }

        bool checkRevisionUserAndTime( ScDocument& rDoc, const OUString& rOwnerName )
        {
            ScChangeTrack* pCT = rDoc.GetChangeTrack();
            if (!pCT)
            {
                cerr << "Change track instance doesn't exist." << endl;
                return false;
            }

            ScChangeAction* pAction = pCT->GetLast();
            if (pAction->GetUser() != "Kohei Yoshida")
            {
                cerr << "Wrong user name." << endl;
                return false;
            }

            DateTime aDT = pAction->GetDateTime();
            if (aDT.GetYear() != 2014 || aDT.GetMonth() != 7 || aDT.GetDay() != 11)
            {
                cerr << "Wrong time stamp." << endl;
                return false;
            }

            // Insert a new record to make sure the user and date-time are correct.
            rDoc.SetString(ScAddress(1,8,0), "New String");
            ScCellValue aEmpty;
            pCT->AppendContent(ScAddress(1,8,0), aEmpty);
            pAction = pCT->GetLast();
            if (!pAction)
            {
                cerr << "Failed to retrieve last revision." << endl;
                return false;
            }

            if (rOwnerName != pAction->GetUser())
            {
                cerr << "Wrong user name." << endl;
                return false;
            }

            DateTime aDTNew = pAction->GetDateTime();
            if (aDTNew <= aDT)
            {
                cerr << "Time stamp of the new revision should be more recent than that of the last revision." << endl;
                return false;
            }

            return true;
        }

    } aTest;

    SvtUserOptions& rUserOpt = SC_MOD()->GetUserOptions();
    rUserOpt.SetToken(UserOptToken::FirstName, "Export");
    rUserOpt.SetToken(UserOptToken::LastName, "Test");

    OUString aOwnerName = rUserOpt.GetFirstName() + " " + rUserOpt.GetLastName();

    // First, test the xls variant.

    ScDocShellRef xDocSh = loadDoc("track-changes/simple-cell-changes.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument* pDoc = &xDocSh->GetDocument();
    bool bGood = aTest.check(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Initial check failed (xls).", bGood);

    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_XLS);
    xDocSh->DoClose();
    pDoc = &xDocSh2->GetDocument();
    bGood = aTest.check(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Check after reload failed (xls).", bGood);

    // fdo#81445 : Check the blank value string to make sure it's "<empty>".
    ScChangeTrack* pCT = pDoc->GetChangeTrack();
    CPPUNIT_ASSERT(pCT);
    ScChangeAction* pAction = pCT->GetAction(1);
    CPPUNIT_ASSERT(pAction);
    OUString aDesc;
    pAction->GetDescription(aDesc, pDoc);
    CPPUNIT_ASSERT_EQUAL(OUString("Cell B2 changed from '<empty>' to '1'"), aDesc);

    bGood = aTest.checkRevisionUserAndTime(*pDoc, aOwnerName);
    CPPUNIT_ASSERT_MESSAGE("Check revision and time failed after reload (xls).", bGood);

    xDocSh2->DoClose();

    // Now, test the xlsx variant the same way.

    xDocSh = loadDoc("track-changes/simple-cell-changes.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    pDoc = &xDocSh->GetDocument();
    aTest.check(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Initial check failed (xlsx).", bGood);

    xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_XLSX);
    xDocSh->DoClose();
    pDoc = &xDocSh2->GetDocument();
    bGood = aTest.check(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Check after reload failed (xlsx).", bGood);

    bGood = aTest.checkRevisionUserAndTime(*pDoc, aOwnerName);
    CPPUNIT_ASSERT_MESSAGE("Check revision and time failed after reload (xlsx).", bGood);

    xDocSh2->DoClose();
}

void ScExportTest::testSheetTabColorsXLSX()
{
    struct
    {
        bool checkContent( const ScDocument& rDoc )
        {

            std::vector<OUString> aTabNames = rDoc.GetAllTableNames();

            // green, red, blue, yellow (from left to right).
            if (aTabNames.size() != 4)
            {
                cerr << "There should be exactly 4 sheets." << endl;
                return false;
            }

            const char* pNames[] = { "Green", "Red", "Blue", "Yellow" };
            for (size_t i = 0; i < SAL_N_ELEMENTS(pNames); ++i)
            {
                OUString aExpected = OUString::createFromAscii(pNames[i]);
                if (aExpected != aTabNames[i])
                {
                    cerr << "incorrect sheet name: expected='" << aExpected <<"', actual='" << aTabNames[i] << "'" << endl;
                    return false;
                }
            }

            const Color aXclColors[] =
            {
                0x0000B050, // green
                0x00FF0000, // red
                0x000070C0, // blue
                0x00FFFF00, // yellow
            };

            for (size_t i = 0; i < SAL_N_ELEMENTS(aXclColors); ++i)
            {
                if (aXclColors[i] != rDoc.GetTabBgColor(i))
                {
                    cerr << "wrong sheet color for sheet " << i << endl;
                    return false;
                }
            }

            return true;
        }

    } aTest;

    ScDocShellRef xDocSh = loadDoc("sheet-tab-color.", FORMAT_XLSX);
    {
        CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocSh.is());
        ScDocument& rDoc = xDocSh->GetDocument();
        bool bRes = aTest.checkContent(rDoc);
        CPPUNIT_ASSERT_MESSAGE("Failed on the initial content check.", bRes);
    }

    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload file.", xDocSh2.is());
    xDocSh->DoClose();
    ScDocument& rDoc = xDocSh2->GetDocument();
    bool bRes = aTest.checkContent(rDoc);
    CPPUNIT_ASSERT_MESSAGE("Failed on the content check after reload.", bRes);

    xDocSh2->DoClose();
}

void ScExportTest::testSharedFormulaExportXLS()
{
    struct
    {
        bool checkContent( ScDocument& rDoc )
        {
            formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1;
            rDoc.SetGrammar(eGram);
            sc::TokenStringContext aCxt(&rDoc, eGram);

            // Check the title row.

            OUString aActual = rDoc.GetString(0,1,0);
            OUString aExpected = "Response";
            if (aActual != aExpected)
            {
                cerr << "Wrong content in A2: expected='" << aExpected << "', actual='" << aActual << "'" << endl;
                return false;
            }

            aActual = rDoc.GetString(1,1,0);
            aExpected = "Response";
            if (aActual != aExpected)
            {
                cerr << "Wrong content in B2: expected='" << aExpected << "', actual='" << aActual << "'" << endl;
                return false;
            }

            // A3:A12 and B3:B12 are numbers from 1 to 10.
            for (SCROW i = 0; i <= 9; ++i)
            {
                double fExpected = i + 1.0;
                ScAddress aPos(0,i+2,0);
                double fActual = rDoc.GetValue(aPos);
                if (fExpected != fActual)
                {
                    cerr << "Wrong value in A" << (i+2) << ": expected=" << fExpected << ", actual=" << fActual << endl;
                    return false;
                }

                aPos.IncCol();
                ScFormulaCell* pFC = rDoc.GetFormulaCell(aPos);
                if (!pFC)
                {
                    cerr << "B" << (i+2) << " should be a formula cell." << endl;
                    return false;
                }

                OUString aFormula = pFC->GetCode()->CreateString(aCxt, aPos);
                aExpected = "Coefficients!RC[-1]";
                if (aFormula != aExpected)
                {
                    cerr << "Wrong formula in B" << (i+2) << ": expected='" << aExpected << "', actual='" << aFormula << "'" << endl;
                    return false;
                }

                fActual = rDoc.GetValue(aPos);
                if (fExpected != fActual)
                {
                    cerr << "Wrong value in B" << (i+2) << ": expected=" << fExpected << ", actual=" << fActual << endl;
                    return false;
                }
            }

            return true;
        }

    } aTest;

    ScDocShellRef xDocSh = loadDoc("shared-formula/3d-reference.", FORMAT_ODS);
    {
        CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocSh.is());
        ScDocument& rDoc = xDocSh->GetDocument();

        // Check the content of the original.
        bool bRes = aTest.checkContent(rDoc);
        CPPUNIT_ASSERT_MESSAGE("Content check on the original document failed.", bRes);
    }

    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_XLS);
    xDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("Failed to reload file.", xDocSh2.is());

    ScDocument& rDoc = xDocSh2->GetDocument();

    // Check the content of the reloaded. This should be identical.
    bool bRes = aTest.checkContent(rDoc);
    CPPUNIT_ASSERT_MESSAGE("Content check on the reloaded document failed.", bRes);

    xDocSh2->DoClose();
}

void ScExportTest::testSharedFormulaExportXLSX()
{
    struct
    {
        bool checkContent( const ScDocument& rDoc )
        {
            SCTAB nTabCount = rDoc.GetTableCount();
            if (nTabCount != 2)
            {
                cerr << "Document should have exactly 2 sheets.  " << nTabCount << " found." << endl;
                return false;
            }

            // Make sure the sheet tab colors are not set.
            for (SCROW i = 0; i <= 1; ++i)
            {
                Color aTabBgColor = rDoc.GetTabBgColor(i);
                if (aTabBgColor != COL_AUTO)
                {
                    cerr << "The tab color of Sheet " << (i+1) << " should not be explicitly set." << endl;
                    return false;
                }
            }

            // B2:B7 should show 1,2,3,4,5,6.
            double fExpected = 1.0;
            for (SCROW i = 1; i <= 6; ++i, ++fExpected)
            {
                ScAddress aPos(1,i,0);
                double fVal = rDoc.GetValue(aPos);
                if (fVal != fExpected)
                {
                    cerr << "Wrong value in B" << (i+1) << ": expected=" << fExpected << ", actual=" << fVal << endl;
                    return false;
                }
            }

            // C2:C7 should show 10,20,....,60.
            fExpected = 10.0;
            for (SCROW i = 1; i <= 6; ++i, fExpected+=10.0)
            {
                ScAddress aPos(2,i,0);
                double fVal = rDoc.GetValue(aPos);
                if (fVal != fExpected)
                {
                    cerr << "Wrong value in C" << (i+1) << ": expected=" << fExpected << ", actual=" << fVal << endl;
                    return false;
                }
            }

            // D2:D7 should show 1,2,...,6.
            fExpected = 1.0;
            for (SCROW i = 1; i <= 6; ++i, ++fExpected)
            {
                ScAddress aPos(3,i,0);
                double fVal = rDoc.GetValue(aPos);
                if (fVal != fExpected)
                {
                    cerr << "Wrong value in D" << (i+1) << ": expected=" << fExpected << ", actual=" << fVal << endl;
                    return false;
                }
            }

            return true;
        }

    } aTest;

    ScDocShellRef xDocSh = loadDoc("shared-formula/3d-reference.", FORMAT_XLSX);
    {
        CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocSh.is());
        ScDocument& rDoc = xDocSh->GetDocument();

        bool bRes = aTest.checkContent(rDoc);
        CPPUNIT_ASSERT_MESSAGE("Content check on the initial document failed.", bRes);

        rDoc.CalcAll(); // Recalculate to flush all cached results.
        bRes = aTest.checkContent(rDoc);
        CPPUNIT_ASSERT_MESSAGE("Content check on the initial recalculated document failed.", bRes);
    }

    // Save and reload, and check the content again.
    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_XLSX);
    xDocSh->DoClose();

    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocSh2.is());
    ScDocument& rDoc = xDocSh2->GetDocument();
    rDoc.CalcAll(); // Recalculate to flush all cached results.

    bool bRes = aTest.checkContent(rDoc);
    CPPUNIT_ASSERT_MESSAGE("Content check on the reloaded document failed.", bRes);

    xDocSh2->DoClose();
}

void ScExportTest::testSharedFormulaStringResultExportXLSX()
{
    struct
    {
        bool checkContent( const ScDocument& rDoc )
        {
            {
                // B2:B7 should show A,B,....,F.
                const char* const expected[] = { "A", "B", "C", "D", "E", "F" };
                for (SCROW i = 0; i <= 5; ++i)
                {
                    ScAddress aPos(1,i+1,0);
                    OUString aStr = rDoc.GetString(aPos);
                    OUString aExpected = OUString::createFromAscii(expected[i]);
                    if (aStr != aExpected)
                    {
                        cerr << "Wrong value in B" << (i+2) << ": expected='" << aExpected << "', actual='" << aStr << "'" << endl;
                        return false;
                    }
                }
            }

            {
                // C2:C7 should show AA,BB,....,FF.
                const char* const expected[] = { "AA", "BB", "CC", "DD", "EE", "FF" };
                for (SCROW i = 0; i <= 5; ++i)
                {
                    ScAddress aPos(2,i+1,0);
                    OUString aStr = rDoc.GetString(aPos);
                    OUString aExpected = OUString::createFromAscii(expected[i]);
                    if (aStr != aExpected)
                    {
                        cerr << "Wrong value in C" << (i+2) << ": expected='" << aExpected << "', actual='" << aStr << "'" << endl;
                        return false;
                    }
                }
            }

            return true;
        }

    } aTest;

    ScDocShellRef xDocSh = loadDoc("shared-formula/text-results.", FORMAT_XLSX);
    {
        CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocSh.is());
        ScDocument& rDoc = xDocSh->GetDocument();

        // Check content without re-calculation, to test cached formula results.
        bool bRes = aTest.checkContent(rDoc);
        CPPUNIT_ASSERT_MESSAGE("Content check on the initial document failed.", bRes);

        // Now, re-calculate and check the results.
        rDoc.CalcAll();
        bRes = aTest.checkContent(rDoc);
        CPPUNIT_ASSERT_MESSAGE("Content check on the initial recalculated document failed.", bRes);
    }
    // Reload and check again.
    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_XLSX);
    xDocSh->DoClose();
    CPPUNIT_ASSERT_MESSAGE("Failed to re-load file.", xDocSh2.is());
    ScDocument& rDoc = xDocSh2->GetDocument();

    bool bRes = aTest.checkContent(rDoc);
    CPPUNIT_ASSERT_MESSAGE("Content check on the reloaded document failed.", bRes);

    xDocSh2->DoClose();
}

void ScExportTest::testFunctionsExcel2010( sal_uLong nFormatType )
{
    ScDocShellRef xShell = loadDoc("functions-excel-2010.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), nFormatType);
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.

    testFunctionsExcel2010_Impl(rDoc);

    xDocSh->DoClose();
}

void ScExportTest::testFunctionsExcel2010XLSX()
{
    testFunctionsExcel2010(FORMAT_XLSX);
}

void ScExportTest::testFunctionsExcel2010XLS()
{
    testFunctionsExcel2010(FORMAT_XLS);
}

void ScExportTest::testCeilingFloor( sal_uLong nFormatType )
{
    ScDocShellRef xShell = loadDoc("ceiling-floor.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), nFormatType);
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.

    testCeilingFloor_Impl(rDoc);

    xDocSh->DoClose();
}

void ScExportTest::testCeilingFloorXLSX()
{
    testCeilingFloor(FORMAT_XLSX);
}

void ScExportTest::testCeilingFloorODSToXLSX()
{
    // tdf#100011 - Cannot open sheet containing FLOOR/CEILING functions by MS Excel, after export to .xlsx
    ScDocShellRef xShell = loadDoc("ceiling-floor.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocPtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/workbook.xml");
    CPPUNIT_ASSERT(pSheet);

    // there shouldn't be any defined names during export of FLOOR and CEILING functions to .xlsx
    assertXPath(pSheet, "/x:workbook/x:definedNames", 0);
}

void ScExportTest::testCeilingFloorXLS()
{
    testCeilingFloor(FORMAT_XLS);
}

void ScExportTest::testCeilingFloorODS()
{
    testCeilingFloor(FORMAT_ODS);
}

void ScExportTest::testCustomXml()
{
    // Load document and export it to a temporary file
    ScDocShellRef xShell = loadDoc("customxml.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocPtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "customXml/item1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    xmlDocPtr pRelsDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "customXml/_rels/item1.xml.rels");
    CPPUNIT_ASSERT(pRelsDoc);

    // Check there is a relation to itemProps1.xml.
    assertXPath(pRelsDoc, "/r:Relationships/r:Relationship", 1);
    assertXPath(pRelsDoc, "/r:Relationships/r:Relationship[@Id='rId1']", "Target", "itemProps1.xml");

    std::shared_ptr<SvStream> pStream = XPathHelper::parseExportStream(pXPathFile, m_xSFactory, "ddp/ddpfile.xen");
    CPPUNIT_ASSERT(pStream);
}

#if !defined _WIN32
void ScExportTest::testRelativePathsODS()
{
    ScDocShellRef xDocSh = loadDoc("fdo79305.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "content.xml", FORMAT_ODS);
    CPPUNIT_ASSERT(pDoc);
    OUString aURL = getXPath(pDoc,
            "/office:document-content/office:body/office:spreadsheet/table:table/table:table-row[2]/table:table-cell[2]/text:p/text:a", "href");
    // make sure that the URL is relative
    CPPUNIT_ASSERT(aURL.startsWith(".."));
}
#endif

namespace {

void testSheetProtection_Impl(const ScDocument& rDoc)
{
    CPPUNIT_ASSERT(rDoc.IsTabProtected(0));
    ScTableProtection* pTabProtection = rDoc.GetTabProtection(0);
    CPPUNIT_ASSERT(pTabProtection);
    CPPUNIT_ASSERT(pTabProtection->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS));
    CPPUNIT_ASSERT(!pTabProtection->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS));
}

}

void ScExportTest::testSheetProtectionODS()
{
    ScDocShellRef xDocSh = loadDoc("sheet-protection.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    {
        ScDocument& rDoc = xDocSh->GetDocument();
        testSheetProtection_Impl(rDoc);
    }

    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_ODS);
    {
        ScDocument& rDoc = xDocSh2->GetDocument();
        testSheetProtection_Impl(rDoc);
    }

    xDocSh2->DoClose();
}

void ScExportTest::testFunctionsExcel2010ODS()
{
    //testFunctionsExcel2010(FORMAT_ODS);
}

void ScExportTest::testSwappedOutImageExport()
{
    const char* aFilterNames[] = {
        "calc8",
        "MS Excel 97",
        "Calc Office Open XML",
        "generic_HTML",
    };

    // Set cache size to a very small value to make sure one of the images is swapped out
    std::shared_ptr< comphelper::ConfigurationChanges > xBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), xBatch);
    xBatch->commit();

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        // Check whether the export code swaps in the image which was swapped out before.
        ScDocShellRef xDocSh = loadDoc("document_with_two_images.", FORMAT_ODS);

        const OString sFailedMessage = OString("Failed on filter: ") + aFilterNames[nFilter];
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xDocSh.is());

        // Export the document and import again for a check
        ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), nFilter);
        xDocSh->DoClose();

        // Check whether graphic exported well after it was swapped out
        uno::Reference< frame::XModel > xModel = xDocSh2->GetModel();
        uno::Reference< sheet::XSpreadsheetDocument > xDoc(xModel, UNO_QUERY_THROW);
        uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
        uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xIA->getByIndex(0), UNO_QUERY_THROW);
        uno::Reference< container::XIndexAccess > xDraws(xDrawPageSupplier->getDrawPage(), UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDraws->getCount());

        uno::Reference<drawing::XShape> xImage(xDraws->getByIndex(0), uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height );
        }
        // Second Image
        xImage.set(xDraws->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height );
        }
        xDocSh2->DoClose();
    }
}

ScExportTest::ScExportTest()
      : ScBootstrapFixture("sc/qa/unit/data")
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

#if !defined _WIN32
void ScExportTest::testSupBookVirtualPathXLS()
{
    ScDocShellRef xShell = loadDoc("external-ref.", FORMAT_XLS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(0,0,0), "'file:///home/timar/Documents/external.xls'#$Sheet1.A1", "Wrong SupBook VirtualPath URL");

    xDocSh->DoClose();
}
#endif

void ScExportTest::testLinkedGraphicRT()
{
    // Problem was with linked images
    const char* aFilterNames[] = {
        "calc8",
        "MS Excel 97",
        "Calc Office Open XML",
        "generic_HTML",
    };

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        // Load the original file with one image
        ScDocShellRef xDocSh = loadDoc("document_with_linked_graphic.", FORMAT_ODS);
        const OString sFailedMessage = OString("Failed on filter: ") + aFilterNames[nFilter];

        // Export the document and import again for a check
        ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), nFilter);
        xDocSh->DoClose();

        // Check whether graphic imported well after export
        ScDocument& rDoc = xDocSh->GetDocument();
        ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pDrawLayer != nullptr );
        const SdrPage *pPage = pDrawLayer->GetPage(0);
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pPage != nullptr );
        SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pObject != nullptr );
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pObject->IsLinkedGraphic() );

        const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), int(GraphicType::Bitmap), int(rGraphicObj.GetGraphic().GetType()));
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), sal_uLong(864900), rGraphicObj.GetGraphic().GetSizeBytes());

        xDocSh2->DoClose();
    }
}

void ScExportTest::testImageWithSpecialID()
{
    const char* aFilterNames[] = {
        "calc8",
        "MS Excel 97",
        "Calc Office Open XML",
        "generic_HTML",
    };

    // Trigger swap out mechanism to test swapped state factor too.
    std::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), batch);
    batch->commit();

    for( size_t nFilter = 0; nFilter < SAL_N_ELEMENTS(aFilterNames); ++nFilter )
    {
        ScDocShellRef xDocSh = loadDoc("images_with_special_IDs.", FORMAT_ODS);

        const OString sFailedMessage = OString("Failed on filter: ") + aFilterNames[nFilter];
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xDocSh.is());

        // Export the document and import again for a check
        ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), nFilter);
        xDocSh->DoClose();

        // Check whether graphic was exported well
        uno::Reference< frame::XModel > xModel = xDocSh2->GetModel();
        uno::Reference< sheet::XSpreadsheetDocument > xDoc(xModel, UNO_QUERY_THROW);
        uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
        uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xIA->getByIndex(0), UNO_QUERY_THROW);
        uno::Reference< container::XIndexAccess > xDraws(xDrawPageSupplier->getDrawPage(), UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2), xDraws->getCount());

        uno::Reference<drawing::XShape> xImage(xDraws->getByIndex(0), uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height );
        }
        // Second Image
        xImage.set(xDraws->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height );
        }
        xDocSh2->DoClose();
    }
}

void ScExportTest::testAbsNamedRangeHTML()
{
    ScDocShellRef xDocSh = loadDoc("numberformat.", FORMAT_HTML);
    xDocSh->DoHardRecalc();
    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_ODS);
    xDocSh->DoClose();
    xDocSh2->DoHardRecalc();

    ScDocument& rDoc = xDocSh2->GetDocument();
    ScRangeData* pRangeData = rDoc.GetRangeName(0)->findByUpperName(OUString("HTML_1"));
    ScSingleRefData* pRef = pRangeData->GetCode()->FirstToken()->GetSingleRef();
    CPPUNIT_ASSERT_MESSAGE("Sheet1.HTML_1 is an absolute reference",!pRef->IsTabRel());
}

void ScExportTest::testSheetLocalRangeNameXLS()
{
    ScDocShellRef xDocSh = loadDoc("named-ranges-local.", FORMAT_XLS);
    xDocSh->DoHardRecalc();
    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_XLS);
    xDocSh->DoClose();
    xDocSh2->DoHardRecalc();

    ScDocument& rDoc = xDocSh2->GetDocument();
    ScRangeName* pRangeName = rDoc.GetRangeName(0);
    CPPUNIT_ASSERT(pRangeName);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pRangeName->size());

    OUString aFormula;
    rDoc.GetFormula(3, 11, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(local_name2)"), aFormula);
    ASSERT_DOUBLES_EQUAL(14.0, rDoc.GetValue(3, 11, 0));

    rDoc.GetFormula(6, 4, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=local_name1"), aFormula);

    xDocSh2->DoClose();
}

void ScExportTest::testRelativeNamedExpressionsXLS()
{
    ScDocShellRef xDocSh = loadDoc("tdf113991_relativeNamedRanges.", FORMAT_ODS);
    xDocSh->DoHardRecalc();
    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_XLS);
    xDocSh->DoClose();
    xDocSh2->DoHardRecalc();
    ScDocument& rDoc = xDocSh2->GetDocument();

    // Sheet1:G3
    ScAddress aPos(6,2,0);
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(aPos));
    ASSERT_FORMULA_EQUAL(rDoc, aPos, "single_cell_A3", nullptr);
    // Sheet2:F6
    aPos = ScAddress(5,5,1);
    CPPUNIT_ASSERT_EQUAL(18.0, rDoc.GetValue(aPos));
    ASSERT_FORMULA_EQUAL(rDoc, aPos, "SUM(test_conflict)", nullptr);
    // Sheet2:H3
    aPos = ScAddress(7,2,1);
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(aPos));
    ASSERT_FORMULA_EQUAL(rDoc, aPos, "single_global_A3", nullptr);
    // Sheet2:H6
    aPos = ScAddress(7,5,1);
    CPPUNIT_ASSERT_EQUAL(75.0, rDoc.GetValue(aPos));
    ASSERT_FORMULA_EQUAL(rDoc, aPos, "SUM(A6:F6)", nullptr);
    xDocSh2->DoClose();
}

void ScExportTest::testSheetTextBoxHyperlinkXLSX()
{
    ScDocShellRef xShell = loadDoc("textbox-hyperlink.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/xdr:wsDr[1]/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:nvSpPr[1]/xdr:cNvPr[1]/a:hlinkClick[1]", 1);

    xDocSh->DoClose();
}

void ScExportTest::testFontSizeXLSX()
{
    ScDocShellRef xDocSh = loadDoc("fontSize.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    OUString fontSize = getXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr", "sz");
    // make sure that the font size is 18
    CPPUNIT_ASSERT_EQUAL(OUString("1800"), fontSize);
}

void ScExportTest::testSheetCharacterKerningSpaceXLSX()
{
    ScDocShellRef xShell = loadDoc("textbox-CharKerningSpace.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    OUString CharKerningSpace = getXPath(pDoc,
        "/xdr:wsDr[1]/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:txBody[1]/a:p[1]/a:r[1]/a:rPr[1]","spc");

    // make sure that the CharKerning is 1997.
    CPPUNIT_ASSERT_EQUAL(OUString("1997"), CharKerningSpace);

    xDocSh->DoClose();
}

void ScExportTest::testSheetCondensedCharacterSpaceXLSX()
{
    ScDocShellRef xShell = loadDoc("textbox-CondensedCharacterSpace.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    OUString CondensedCharSpace = getXPath(pDoc,
        "/xdr:wsDr[1]/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:txBody[1]/a:p[1]/a:r[1]/a:rPr[1]","spc");

    // make sure that the CondensedCharSpace is -1002.
    CPPUNIT_ASSERT_EQUAL(OUString("-1002"), CondensedCharSpace);

    xDocSh->DoClose();
}

void ScExportTest::testTextUnderlineColorXLSX()
{
    ScDocShellRef xDocSh = loadDoc("underlineColor.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    // Make sure the underline type is double line
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr", "u", "dbl");

    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr", "b", "1");
    // Make sure that the underline color is RED
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr/a:uFill/a:solidFill/a:srgbClr", "val", "ff0000");

    // Make sure the underline type is drawn with heavy line
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr", "u", "heavy");
    // tdf#104219 Make sure that uFill is not existing and uFillTx is set.
    // It mean that color is automatic, should be the same color as the text.
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr/a:uFill", 0);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr/a:uFillTx", 1);
}

void ScExportTest::testSheetRunParagraphPropertyXLSX()
{
    ScDocShellRef xShell = loadDoc("TextColor.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/sharedStrings.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    OUString aColor = getXPath(pDoc, "/x:sst/x:si/x:r[1]/x:rPr[1]/x:color", "rgb");
    CPPUNIT_ASSERT_EQUAL(OUString("FFFF0000"), aColor);

    xDocSh->DoClose();
}

void ScExportTest::testPreserveTextWhitespaceXLSX()
{
    ScDocShellRef xShell = loadDoc("preserve-whitespace.", FORMAT_XLSX);
    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/sharedStrings.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:sst/x:si/x:t", "space", "preserve");
    xDocSh->DoClose();
}

void ScExportTest::testPreserveTextWhitespace2XLSX()
{
    ScDocShellRef xShell = loadDoc("preserve_space.", FORMAT_XLSX);
    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/sharedStrings.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:sst/x:si[1]/x:t", "space", "preserve");
    assertXPath(pDoc, "/x:sst/x:si[2]/x:r[1]/x:t", "space", "preserve");
    assertXPath(pDoc, "/x:sst/x:si[2]/x:r[2]/x:t", "space", "preserve");
    xDocSh->DoClose();
}

void ScExportTest::testHiddenShapeXLSX()
{
    ScDocShellRef xDocSh = loadDoc("hiddenShape.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp[1]/xdr:nvSpPr/xdr:cNvPr", "hidden", "1");
}

void ScExportTest::testHyperlinkXLSX()
{
    ScDocShellRef xDocSh = loadDoc("hyperlink.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/_rels/drawing1.xml.rels", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/r:Relationships/r:Relationship", "Target", "#Sheet2!A1");
}

void ScExportTest::testMoveCellAnchoredShapesODS()
{
    ScDocShellRef xDocSh = loadDoc("move-cell-anchored-shapes.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load move-cell-anchored-shapes.ods", xDocSh.is());

    // There are two cell-anchored objects on the first sheet.
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", rDoc.GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);

    // Check cell anchor state
    ScAnchorType oldType = ScDrawLayer::GetAnchorType(*pObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Failed to get anchor type", SCA_CELL_RESIZE, oldType);

    // Get anchor data
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pData->getShapeRect().IsEmpty());

    ScAddress aDataStart = pData->maStart;
    ScAddress aDataEnd   = pData->maEnd;

    // Get non rotated anchor data
    ScDrawObjData* pNData = ScDrawLayer::GetNonRotatedObjData( pObj );
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pNData->getShapeRect().IsEmpty());

    ScAddress aNDataStart = pNData->maStart;
    ScAddress aNDataEnd   = pNData->maEnd;
    CPPUNIT_ASSERT_EQUAL(aDataStart, aNDataStart);
    CPPUNIT_ASSERT_EQUAL(aDataEnd , aNDataEnd);

    // Insert 2 rows.
    rDoc.InsertRow(ScRange( 0, aDataStart.Row() - 1, 0, MAXCOL, aDataStart.Row(), 0));

    // Get anchor data
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pData->getShapeRect().IsEmpty());

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData( pObj );
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pNData->getShapeRect().IsEmpty());

    // Check if data has moved to new rows
    CPPUNIT_ASSERT_EQUAL( pData->maStart.Row(), aDataStart.Row() + 2 );
    CPPUNIT_ASSERT_EQUAL( pData->maEnd.Row(), aDataEnd.Row() + 2 );

    CPPUNIT_ASSERT_EQUAL( pNData->maStart.Row(), aNDataStart.Row() + 2 );
    CPPUNIT_ASSERT_EQUAL( pNData->maEnd.Row(), aNDataEnd.Row() + 2 );

    // Save the anchor data
    aDataStart = pData->maStart;
    aDataEnd   = pData->maEnd;
    aNDataStart = pNData->maStart;
    aNDataEnd   = pNData->maEnd;

    // Save the document and load again to check anchor persist
    ScDocShellRef xDocSh1 = saveAndReload(&(*xDocSh), FORMAT_ODS);

    // There are two cell-anchored objects on the first sheet.
    ScDocument& rDoc1 = xDocSh1->GetDocument();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", rDoc1.GetTableCount() > 0);

    pDrawLayer = rDoc1.GetDrawLayer();
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);

    // Check cell anchor state
    oldType = ScDrawLayer::GetAnchorType(*pObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Failed to get anchor type", SCA_CELL_RESIZE, oldType);

    // Get anchor data
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pData->getShapeRect().IsEmpty());

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData( pObj );
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pNData->getShapeRect().IsEmpty());

    // Check if data after save it
    CPPUNIT_ASSERT_EQUAL(pData->maStart, aDataStart);
    CPPUNIT_ASSERT_EQUAL(pData->maEnd , aDataEnd);

    CPPUNIT_ASSERT_EQUAL(pNData->maStart, aNDataStart);
    CPPUNIT_ASSERT_EQUAL(pNData->maEnd , aNDataEnd);

    // Insert a column.
    rDoc1.InsertCol(ScRange( aDataStart.Col(), 0 , 0 , aDataStart.Col(), MAXROW, 0 ));

    // Get anchor data
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pData->getShapeRect().IsEmpty());

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData( pObj );
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pNData->getShapeRect().IsEmpty());

    // Check if data has moved to new rows
    CPPUNIT_ASSERT_EQUAL(pData->maStart.Col(), SCCOL(aDataStart.Col() + 1));
    CPPUNIT_ASSERT_EQUAL(pData->maEnd.Col() , SCCOL(aDataEnd.Col() + 1));

    CPPUNIT_ASSERT_EQUAL(pNData->maStart.Col(), SCCOL(aNDataStart.Col() + 1));
    CPPUNIT_ASSERT_EQUAL(pNData->maEnd.Col() , SCCOL(aNDataEnd.Col() + 1));

    // Save the anchor data
    aDataStart = pData->maStart;
    aDataEnd   = pData->maEnd;
    aNDataStart = pNData->maStart;
    aNDataEnd   = pNData->maEnd;

    // Save the document and load again to check anchor persist
    ScDocShellRef xDocSh2 = saveAndReload(&(*xDocSh1), FORMAT_ODS);

    // There are two cell-anchored objects on the first sheet.
    ScDocument& rDoc2 = xDocSh2->GetDocument();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", rDoc2.GetTableCount() > 0);

    pDrawLayer = rDoc2.GetDrawLayer();
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);

    // Check cell anchor state
    oldType = ScDrawLayer::GetAnchorType(*pObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Failed to get anchor type", SCA_CELL_RESIZE, oldType);

    // Get anchor data
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pData->getShapeRect().IsEmpty());

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData( pObj );
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pNData->getShapeRect().IsEmpty());

    // Check if data after save it
    CPPUNIT_ASSERT_EQUAL(pData->maStart, aDataStart);
    CPPUNIT_ASSERT_EQUAL(pData->maEnd , aDataEnd);

    CPPUNIT_ASSERT_EQUAL(pNData->maStart, aNDataStart);
    CPPUNIT_ASSERT_EQUAL(pNData->maEnd , aNDataEnd);

    xDocSh2->DoClose();
}

void ScExportTest::testMatrixMultiplicationXLSX()
{
    ScDocShellRef xShell = loadDoc("matrix-multiplication.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    OUString CellFormulaRange = getXPath(pDoc,
        "/x:worksheet/x:sheetData/x:row[4]/x:c/x:f","ref");

    // make sure that the CellFormulaRange is G5:G6.
    CPPUNIT_ASSERT_EQUAL(OUString("G5:G6"), CellFormulaRange);

    OUString CellFormulaType = getXPath(pDoc,
        "/x:worksheet/x:sheetData/x:row[4]/x:c/x:f","t");

    // make sure that the CellFormulaType is array.
    CPPUNIT_ASSERT_EQUAL(OUString("array"), CellFormulaType);

    xDocSh->DoClose();
}

void ScExportTest::testRefStringXLSX()
{
    ScDocShellRef xDocSh = loadDoc("ref_string.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    //make sure ref syntax gets saved for MSO-produced docs
    xDocSh = saveAndReload( &(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScCalcConfig aCalcConfig = rDoc.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL(formula::FormulaGrammar::CONV_XL_A1, aCalcConfig.meStringRefAddressSyntax);

    xDocSh->DoClose();
}

void ScExportTest::testRefStringConfigXLSX()
{
    // this doc is configured with CalcA1 ref syntax
    ScDocShellRef xDocSh = loadDoc("empty.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    xDocSh = saveAndReload( &(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScCalcConfig aConfig = rDoc.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match", formula::FormulaGrammar::CONV_OOO,
                            aConfig.meStringRefAddressSyntax);

    xDocSh->DoClose();

    // this doc has no entry for ref syntax
    xDocSh = loadDoc("empty-noconf.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open 2nd doc", xDocSh.is());

    ScDocument& rDoc2 = xDocSh->GetDocument();
    aConfig = rDoc2.GetCalcConfig();
    // therefore after import, ref syntax should be set to CalcA1 | ExcelA1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match", formula::FormulaGrammar::CONV_A1_XL_A1,
                            aConfig.meStringRefAddressSyntax);

    //set ref syntax to something else than ExcelA1 (native to xlsx format) ...
    aConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_XL_R1C1;
    rDoc2.SetCalcConfig( aConfig );

    ScDocShellRef xNewDocSh = saveAndReload( &(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload 2nd doc", xNewDocSh.is());

    // ... and make sure it got saved
    ScDocument& rDoc3 = xNewDocSh->GetDocument();
    aConfig = rDoc3.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match", formula::FormulaGrammar::CONV_XL_R1C1,
                            aConfig.meStringRefAddressSyntax);

    xDocSh->DoClose();
    xNewDocSh->DoClose();
}

void ScExportTest::testRefStringUnspecified()
{
    ScDocShell* pShell = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    pShell->DoInitNew();

    ScDocument& rDoc = pShell->GetDocument();
    ScCalcConfig aConfig = rDoc.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default string ref syntax value doesn't match", formula::FormulaGrammar::CONV_UNSPECIFIED,
                            aConfig.meStringRefAddressSyntax);

    // change formula syntax (i.e. not string ref syntax) to ExcelA1
    rDoc.SetGrammar( formula::FormulaGrammar::GRAM_NATIVE_XL_A1 );

    ScDocShellRef xDocSh = saveAndReload(pShell, FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload doc", xDocSh.is());

    // with string ref syntax at its default value, we should've saved ExcelA1
    ScDocument& rDoc2 = xDocSh->GetDocument();
    aConfig = rDoc2.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match", formula::FormulaGrammar::CONV_XL_A1,
                            aConfig.meStringRefAddressSyntax);

    xDocSh->DoClose();
}

void ScExportTest::testHeaderImageODS()
{
    // Graphic as header background was lost on export.
    ScDocShellRef xShell = loadDoc("header-image.", FORMAT_ODS);
    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_ODS);
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xDocSh->GetModel(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xPageStyles(xStyleFamilies->getByName("PageStyles"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName("Default"), uno::UNO_QUERY);

    uno::Reference<graphic::XGraphic> xGraphic;
    xStyle->getPropertyValue("HeaderBackGraphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    xDocSh->DoClose();
}

void ScExportTest::testTextDirectionXLSX()
{
    ScDocShellRef xDocSh = loadDoc("writingMode.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment", "readingOrder", "1");//LTR
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[3]/x:alignment", "readingOrder", "2");//RTL
}

void ScExportTest::testTdf88657ODS()
{
    ScDocShellRef xDocSh = loadDoc("tdf88657.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "styles.xml", FORMAT_ODS);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//number:fraction", "min-denominator-digits", "3");
}

void ScExportTest::testConditionalFormatRangeListXLSX()
{
    ScDocShellRef xDocSh = loadDoc("conditionalformat_rangelist.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:conditionalFormatting", "sqref", "F4 F10");
}

void ScExportTest::testConditionalFormatContainsTextXLSX()
{
    ScDocShellRef xDocSh = loadDoc("conditionalformat_containstext.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "//x:conditionalFormatting/x:cfRule/x:formula", "NOT(ISERROR(SEARCH(\"test\",A1)))");
}

void ScExportTest::testEscapeCharInNumberFormatXLSX()
{
    ScDocShellRef xDocSh = loadDoc("tdf81939.", FORMAT_XLSX);
    CPPUNIT_ASSERT( xDocSh.is() );
    xDocSh = saveAndReload( &(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT( xDocSh.is() );

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    const sal_Unicode cEuro (8364);  // € symbol
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode", "00\\ 00\\ 00\\ 00\\ 00");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode", "00\\.00\\.00\\.000\\.0");   // tdf#81939
    // "_-* #,##0\ _€_-;\-* #,##0\ _€_-;_-* "- "_€_-;_-@_-" // tdf#81222
    OUString rFormatStrExpected ( "_-* #,##0\\ _" + OUStringLiteral1(cEuro) + "_-;\\-* #,##0\\ _" +
            OUStringLiteral1(cEuro) + "_-;_-* \"- \"_" + OUStringLiteral1(cEuro) + "_-;_-@_-" );
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[4]", "formatCode", rFormatStrExpected );
    // "_-* #,##0" €"_-;\-* #,##0" €"_-;_-* "- €"_-;_-@_-");
    rFormatStrExpected = "_-* #,##0\" " + OUStringLiteral1(cEuro) + "\"_-;\\-* #,##0\" " +
            OUStringLiteral1(cEuro) + "\"_-;_-* \"- " + OUStringLiteral1(cEuro) + "\"_-;_-@_-";
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[5]", "formatCode", rFormatStrExpected );
    // remove escape char in fraction
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[6]", "formatCode", "# ?/?;[RED]\\-# #/#####");

    xDocSh->DoClose();
}

void ScExportTest::testNatNumInNumberFormatXLSX()
{
    ScDocShellRef xDocSh = loadDoc("tdf79398_NatNum5.", FORMAT_ODS);
    CPPUNIT_ASSERT( xDocSh.is() );
    xDocSh = saveAndReload( &(*xDocSh), FORMAT_XLSX);  // Convert [NatNum5] to [DBNum2] in Chinese
    CPPUNIT_ASSERT( xDocSh.is() );

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode", "[DBNum2][$-804]General;[RED][DBNum2][$-804]General");

    xDocSh->DoClose();
}

void ScExportTest::testExponentWithoutSignFormatXLSX()
{
    ScDocShellRef xDocSh = loadDoc("tdf102370_ExponentWithoutSign.", FORMAT_ODS);
    CPPUNIT_ASSERT( xDocSh.is() );
    xDocSh = saveAndReload( &(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT( xDocSh.is() );

    xDocSh = saveAndReload( &(*xDocSh), FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    sal_uInt32 nNumberFormat;
    rDoc.GetNumberFormat(0, 0, 0, nNumberFormat);
    const SvNumberformat* pNumberFormat = rDoc.GetFormatTable()->GetEntry(nNumberFormat);
    const OUString& rFormatStr = pNumberFormat->GetFormatstring();
    const OUString aExpectedFormatStr = "0.00E0";

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format lost exponent without sign during Excel export", aExpectedFormatStr, rFormatStr);

    xDocSh->DoClose();
}

void ScExportTest::testExtendedLCIDXLSX()
{
    ScDocShellRef xDocSh = loadDoc("tdf36038_ExtendedLCID.", FORMAT_ODS);
    CPPUNIT_ASSERT( xDocSh.is() );
    xDocSh = saveAndReload( &(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT( xDocSh.is() );

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    // Check export
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode", "[$-107041E]DD\\-MM\\-YYYY");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode", "[$-D07041E]DD\\-MM\\-YYYY");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[4]", "formatCode", "[$-1030411]DD\\-MM\\-EE");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[5]", "formatCode", "[$-1B030411]DD\\-MM\\-EE");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[6]", "formatCode", "[$-108040D]DD\\-MM\\-YYYY");
    //assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[7]", "formatCode", "[$-108040D]DD\\-MM\\-YYYY");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[7]", "formatCode", "[$-1060401]DD\\-MM\\-YYYY");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[8]", "formatCode", "[$-2060401]DD\\-MM\\-YYYY");

    // Check import
    ScDocument& rDoc = xDocSh->GetDocument();
    SvNumberFormatter* pNumFormatter = rDoc.GetFormatTable();
    sal_uInt32 nNumberFormat;
    const OUString aLang[4] = { "[$-41E]", "[$-411]", "[$-40D]", "[$-401]" };
    const OUString aCalendar[4] = { "[~buddhist]DD-MM-YYYY", "[~gengou]DD-MM-EE", "[~jewish]DD-MM-YYYY", "[~hijri]DD-MM-YYYY" };
    for ( sal_Int16 nCol = 1; nCol <= 2; nCol++ )
    {
        for ( sal_Int16 nRow = 1; nRow <= 4; nRow++ )
        {
            rDoc.GetNumberFormat(nCol, nRow, 0, nNumberFormat);
            const SvNumberformat* pNumberFormat = pNumFormatter->GetEntry(nNumberFormat);
            const OUString& rFormatStr = pNumberFormat->GetFormatstring();
            const OUString aExpectedFormatStr = aLang[nRow-1] + ( (nCol==2 && nRow!=3) ? OUString("[NatNum1]") : OUString() ) + aCalendar[nRow-1];

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format lost extended LCID during Excel export", aExpectedFormatStr, rFormatStr);
        }
    }

    xDocSh->DoClose();
}

void ScExportTest::testHiddenRepeatedRowsODS()
{
    ScDocShellRef xDocSh = loadDoc("empty.", FORMAT_ODS);
    CPPUNIT_ASSERT( xDocSh.is() );

    {
        ScDocument& rDoc = xDocSh->GetDocument();
        rDoc.SetRowHidden(0, 20, 0, true);
    }

    xDocSh = saveAndReload( &(*xDocSh), FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    SCROW nFirstRow = 0;
    SCROW nLastRow = 0;
    bool bHidden = rDoc.RowHidden(0, 0, &nFirstRow, &nLastRow);
    CPPUNIT_ASSERT(bHidden);
    CPPUNIT_ASSERT_EQUAL(SCROW(0), nFirstRow);
    CPPUNIT_ASSERT_EQUAL(SCROW(20), nLastRow);
    xDocSh->DoClose();
}

void ScExportTest::testHyperlinkTargetFrameODS()
{
    ScDocShellRef xDocSh = loadDoc("hyperlink_frame.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const EditTextObject* pEditText = rDoc.GetEditText(ScAddress(2, 5, 0));
    CPPUNIT_ASSERT(pEditText);

    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the URL data.", pData && pData->GetClassId() == text::textfield::Type::URL);

    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    OUString aTargetFrame = pURLData->GetTargetFrame();
    CPPUNIT_ASSERT_EQUAL(OUString("_blank"), aTargetFrame);

    xmlDocPtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "content.xml", FORMAT_ODS);
    CPPUNIT_ASSERT(pDoc);
    OUString aTargetFrameExport = getXPath(pDoc,
            "/office:document-content/office:body/office:spreadsheet/table:table/table:table-row[2]/table:table-cell[2]/text:p/text:a", "target-frame-name");
    CPPUNIT_ASSERT_EQUAL(OUString("_blank"), aTargetFrameExport);
}

void ScExportTest::testOpenDocumentAsReadOnly()
{
    ScDocShellRef xDocSh = loadDoc("open-as-read-only.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh->IsSecurityOptOpenReadOnly());
    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh2->IsSecurityOptOpenReadOnly());
    xDocSh->DoClose();
    xDocSh2->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
