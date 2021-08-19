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
#include <config_features.h>

#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/docfilt.hxx>

#include "helper/debughelper.hxx"
#include "helper/qahelper.hxx"
#include "helper/xpath.hxx"
#include "helper/shared_test_impl.hxx"

#include <userdat.hxx>
#include <docsh.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <stlpool.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <tokenarray.hxx>
#include <editutil.hxx>
#include <scopetools.hxx>
#include <cellvalue.hxx>
#include <postit.hxx>
#include <tokenstringcontext.hxx>
#include <chgtrack.hxx>
#include <validat.hxx>
#include <global.hxx>
#include <scmod.hxx>
#include <dpcache.hxx>
#include <dpobject.hxx>
#include <clipparam.hxx>

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
#include <comphelper/scopeguard.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <tools/datetime.hxx>
#include <tools/fldunit.hxx>
#include <svl/zformat.hxx>

#include <test/xmltesttools.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/sheet/GlobalSheetSettings.hpp>

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

    void test();
    void testTdf139167();
    void testTdf139394();
    void testExtCondFormatXLSX();
    void testTdf90104();
    void testTdf111876();
    void testPasswordExportODS();
    void testTdf134332();
    void testConditionalFormatExportODS();
    void testConditionalFormatExportXLSX();
    void testCondFormatExportCellIs();
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
    void testConditionalFormatPriorityCheckXLSX();
    void testConditionalFormatOriginXLSX();
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
    void testCommentExportXLSX_2_XLSX();
#if HAVE_MORE_FONTS
    void testCustomColumnWidthExportXLSX();
#endif
    void testXfDefaultValuesXLSX();
    void testODF13();
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

    void testRelativePathsODS();
    void testSheetProtectionODS();

    void testSwappedOutImageExport();
    void testLinkedGraphicRT();
    void testImageWithSpecialID();

    void testSupBookVirtualPathXLS();
    void testAbsNamedRangeHTML();
    void testSheetLocalRangeNameXLS();
    void testRelativeNamedExpressionsXLS();
    void testSheetTextBoxHyperlinkXLSX();
    void testFontSizeXLSX();
    void testSheetCharacterKerningSpaceXLSX();
    void testSheetCondensedCharacterSpaceXLSX();
    void testTextUnderlineColorXLSX();
    void testSheetRunParagraphPropertyXLSX();
    void testHiddenShapeXLS();
    void testHiddenShapeXLSX();
    void testShapeAutofitXLSX();
    void testHyperlinkXLSX();
    void testMoveCellAnchoredShapesODS();
    void testMatrixMultiplicationXLSX();
    void testPreserveTextWhitespaceXLSX();
    void testPreserveTextWhitespace2XLSX();
    void testTextDirectionXLSX();
    void testTdf66668();
    void testTdf130108();
    void testTdf76949();
    void testTdf55417();
    void testTdf129985();
    void testTdf73063();

    xmlDocUniquePtr testTdf95640(const OUString& rFileName, sal_Int32 nSourceFormat,
                                 sal_Int32 nDestFormat);
    void testTdf95640_ods_to_xlsx();
    void testTdf95640_ods_to_xlsx_with_standard_list();
    void testTdf95640_xlsx_to_xlsx();

    void testRefStringXLSX();
    void testRefStringConfigXLSX();
    void testRefStringUnspecified();
    void testHeaderImageODS();

    void testTdf88657ODS();
    void testTdf41722();
    void testTdf113621();
    void testEscapeCharInNumberFormatXLSX();
    void testNatNumInNumberFormatXLSX();
    void testExponentWithoutSignFormatXLSX();
    void testExtendedLCIDXLSX();

    void testHiddenRepeatedRowsODS();
    void testHyperlinkTargetFrameODS();
    void testOpenDocumentAsReadOnly();
    void testKeepSettingsOfBlankRows();

    void testTdf133595();
    void testTdf134769();
    void testTdf106181();
    void testTdf105272();
    void testTdf118990();
    void testTdf121612();
    void testTdf112936();
    void testPivotCacheAfterExportXLSX();
    void testTdf114969XLSX();
    void testTdf115192XLSX();
    void testTdf91634XLSX();
    void testTdf115159();
    void testTdf112567();
    void testTdf112567b();
    void testTdf123645XLSX();
    void testTdf125173XLSX();
    void testTdf79972XLSX();
    void testTdf126024XLSX();
    void testTdf126177XLSX();
    void testCommentTextVAlignment();
    void testCommentTextHAlignment();
    void testValidationCopyPaste();

    void testXltxExport();
    void testRotatedImageODS();
    void testTdf128976();
    void testTdf120502();
    void testTdf131372();
    void testTdf81470();
    void testTdf122331();
    void testTdf83779();
    void testTdf121716_ExportEvenHeaderFooterXLSX();
    void testTdf134459_HeaderFooterColorXLSX();
    void testTdf134817_HeaderFooterTextWith2SectionXLSX();
    void testTdf121718_UseFirstPageNumberXLSX();
    void testHeaderFontStyleXLSX();
    void testTdf135828_Shape_Rect();
    void testTdf123353();
    void testTdf133688_precedents();
    void testTdf91251_missingOverflowRoundtrip();
    void testTdf137000_handle_upright();
    void testTdf126305_DataValidatyErrorAlert();
    void testTdf76047_externalLink();
    void testTdf87973_externalLinkSkipUnuseds();
    void testTdf51022_lostPrintRange();
    void testTdf138741_externalLinkSkipUnusedsCrash();
    void testTdf138824_linkToParentDirectory();
    void testTdf129969();
    void testTdf84874();
    void testTdf136721_paper_size();
    void testTdf139258_rotated_image();
    void testTdf140431();
    void testTdf142264ManyChartsToXLSX();

    CPPUNIT_TEST_SUITE(ScExportTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(testTdf139167);
    CPPUNIT_TEST(testTdf139394);
    CPPUNIT_TEST(testExtCondFormatXLSX);
    CPPUNIT_TEST(testTdf90104);
    CPPUNIT_TEST(testTdf111876);
    CPPUNIT_TEST(testPasswordExportODS);
    CPPUNIT_TEST(testTdf134332);
    CPPUNIT_TEST(testConditionalFormatExportODS);
    CPPUNIT_TEST(testCondFormatExportCellIs);
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
    CPPUNIT_TEST(testConditionalFormatPriorityCheckXLSX);
    CPPUNIT_TEST(testConditionalFormatOriginXLSX);
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
    CPPUNIT_TEST(testCommentExportXLSX_2_XLSX);
#if HAVE_MORE_FONTS
    CPPUNIT_TEST(testCustomColumnWidthExportXLSX);
#endif
    CPPUNIT_TEST(testXfDefaultValuesXLSX);
    CPPUNIT_TEST(testODF13);
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
    CPPUNIT_TEST(testRelativePathsODS);
    CPPUNIT_TEST(testSheetProtectionODS);
    CPPUNIT_TEST(testSupBookVirtualPathXLS);
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
    CPPUNIT_TEST(testHiddenShapeXLS);
    CPPUNIT_TEST(testHiddenShapeXLSX);
    CPPUNIT_TEST(testShapeAutofitXLSX);
    CPPUNIT_TEST(testHyperlinkXLSX);
    CPPUNIT_TEST(testMoveCellAnchoredShapesODS);
    CPPUNIT_TEST(testMatrixMultiplicationXLSX);
    CPPUNIT_TEST(testTextDirectionXLSX);
    CPPUNIT_TEST(testTdf66668);
    CPPUNIT_TEST(testTdf130108);
    CPPUNIT_TEST(testTdf76949);
    CPPUNIT_TEST(testTdf55417);
    CPPUNIT_TEST(testTdf129985);
    CPPUNIT_TEST(testTdf73063);
    CPPUNIT_TEST(testTdf95640_ods_to_xlsx);
    CPPUNIT_TEST(testTdf95640_ods_to_xlsx_with_standard_list);
    CPPUNIT_TEST(testTdf95640_xlsx_to_xlsx);

    CPPUNIT_TEST(testRefStringXLSX);
    CPPUNIT_TEST(testRefStringConfigXLSX);
    CPPUNIT_TEST(testRefStringUnspecified);
    CPPUNIT_TEST(testHeaderImageODS);

    CPPUNIT_TEST(testTdf88657ODS);
    CPPUNIT_TEST(testTdf41722);
    CPPUNIT_TEST(testTdf113621);
    CPPUNIT_TEST(testEscapeCharInNumberFormatXLSX);
    CPPUNIT_TEST(testNatNumInNumberFormatXLSX);
    CPPUNIT_TEST(testExponentWithoutSignFormatXLSX);
    CPPUNIT_TEST(testExtendedLCIDXLSX);

    CPPUNIT_TEST(testHiddenRepeatedRowsODS);
    CPPUNIT_TEST(testHyperlinkTargetFrameODS);
    CPPUNIT_TEST(testOpenDocumentAsReadOnly);
    CPPUNIT_TEST(testKeepSettingsOfBlankRows);

    CPPUNIT_TEST(testTdf133595);
    CPPUNIT_TEST(testTdf134769);
    CPPUNIT_TEST(testTdf106181);
    CPPUNIT_TEST(testTdf105272);
    CPPUNIT_TEST(testTdf118990);
    CPPUNIT_TEST(testTdf121612);
    CPPUNIT_TEST(testTdf112936);
    CPPUNIT_TEST(testPivotCacheAfterExportXLSX);
    CPPUNIT_TEST(testTdf114969XLSX);
    CPPUNIT_TEST(testTdf115192XLSX);
    CPPUNIT_TEST(testTdf91634XLSX);
    CPPUNIT_TEST(testTdf115159);
    CPPUNIT_TEST(testTdf112567);
    CPPUNIT_TEST(testTdf112567b);
    CPPUNIT_TEST(testTdf123645XLSX);
    CPPUNIT_TEST(testTdf125173XLSX);
    CPPUNIT_TEST(testTdf79972XLSX);
    CPPUNIT_TEST(testTdf126024XLSX);
    CPPUNIT_TEST(testTdf126177XLSX);
    CPPUNIT_TEST(testCommentTextVAlignment);
    CPPUNIT_TEST(testCommentTextHAlignment);
    CPPUNIT_TEST(testValidationCopyPaste);

    CPPUNIT_TEST(testXltxExport);
    CPPUNIT_TEST(testRotatedImageODS);
    CPPUNIT_TEST(testTdf128976);
    CPPUNIT_TEST(testTdf120502);
    CPPUNIT_TEST(testTdf131372);
    CPPUNIT_TEST(testTdf81470);
    CPPUNIT_TEST(testTdf122331);
    CPPUNIT_TEST(testTdf83779);
    CPPUNIT_TEST(testTdf121716_ExportEvenHeaderFooterXLSX);
    CPPUNIT_TEST(testTdf134459_HeaderFooterColorXLSX);
    CPPUNIT_TEST(testTdf134817_HeaderFooterTextWith2SectionXLSX);
    CPPUNIT_TEST(testTdf121718_UseFirstPageNumberXLSX);
    CPPUNIT_TEST(testHeaderFontStyleXLSX);
    CPPUNIT_TEST(testTdf135828_Shape_Rect);
    CPPUNIT_TEST(testTdf123353);
    CPPUNIT_TEST(testTdf133688_precedents);
    CPPUNIT_TEST(testTdf91251_missingOverflowRoundtrip);
    CPPUNIT_TEST(testTdf137000_handle_upright);
    CPPUNIT_TEST(testTdf126305_DataValidatyErrorAlert);
    CPPUNIT_TEST(testTdf76047_externalLink);
    CPPUNIT_TEST(testTdf87973_externalLinkSkipUnuseds);
    CPPUNIT_TEST(testTdf51022_lostPrintRange);
    CPPUNIT_TEST(testTdf138741_externalLinkSkipUnusedsCrash);
    CPPUNIT_TEST(testTdf138824_linkToParentDirectory);
    CPPUNIT_TEST(testTdf129969);
    CPPUNIT_TEST(testTdf84874);
    CPPUNIT_TEST(testTdf136721_paper_size);
    CPPUNIT_TEST(testTdf139258_rotated_image);
    CPPUNIT_TEST(testTdf140431);
    CPPUNIT_TEST(testTdf142264ManyChartsToXLSX);

    CPPUNIT_TEST_SUITE_END();

private:
    void testExcelCellBorders( sal_uLong nFormatType );

    uno::Reference<uno::XInterface> m_xCalcComponent;

};

void ScExportTest::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    static const struct { xmlChar* pPrefix; xmlChar* pURI; } aNamespaces[] =
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
        { BAD_CAST("xx"), BAD_CAST("urn:schemas-microsoft-com:office:excel") },
        { BAD_CAST("r"), BAD_CAST("http://schemas.openxmlformats.org/package/2006/relationships") },
        { BAD_CAST("number"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0") },
        { BAD_CAST("loext"), BAD_CAST("urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0") },
        { BAD_CAST("tableooo"), BAD_CAST("http://openoffice.org/2009/table") },
        { BAD_CAST("ContentType"), BAD_CAST("http://schemas.openxmlformats.org/package/2006/content-types") },
        { BAD_CAST("x14"), BAD_CAST("http://schemas.microsoft.com/office/spreadsheetml/2009/9/main") },
        { BAD_CAST("xm"), BAD_CAST("http://schemas.microsoft.com/office/excel/2006/main") },
        { BAD_CAST("x12ac"), BAD_CAST("http://schemas.microsoft.com/office/spreadsheetml/2011/1/ac") },
    };
    for(size_t i = 0; i < SAL_N_ELEMENTS(aNamespaces); ++i)
    {
        xmlXPathRegisterNs(pXmlXPathCtx, aNamespaces[i].pPrefix, aNamespaces[i].pURI );
    }
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

void ScExportTest::testTdf139167()
{
    ScDocShellRef xShell = loadDoc(u"tdf139167.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/styles.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:cellStyles", "count", "6");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:fill/x:patternFill/x:bgColor", "rgb",
                "FFFFFF00");

    xDocSh->DoClose();
}

void ScExportTest::testTdf139394()
{
    ScDocShellRef xShell = loadDoc(u"tdf139394.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc,
                "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[1]/"
                "x14:cfRule/xm:f", "LEFT(A1,LEN(\"+\"))=\"+\"");
    assertXPathContent(pDoc,
                "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[2]/"
                "x14:cfRule/xm:f", "RIGHT(A2,LEN(\"-\"))=\"-\"");
    assertXPathContent(pDoc,
                "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[3]/"
                "x14:cfRule/xm:f", "LEFT(A3,LEN($B$3))=$B$3");

    xDocSh->DoClose();
}

void ScExportTest::testExtCondFormatXLSX()
{
    ScDocShellRef xShell = loadDoc("tdf139021.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc,
                "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[1]/"
                "x14:cfRule", "type", "containsText");
    assertXPathContent(pDoc,
                "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[1]/"
                "x14:cfRule/xm:f[1]", "NOT(ISERROR(SEARCH($B$1,A1)))");
    assertXPathContent(pDoc,
                "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[1]/"
                "x14:cfRule/xm:f[2]", "$B$1");
    assertXPath(pDoc,
                "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[2]/"
                "x14:cfRule", "type", "notContainsText");
    assertXPathContent(pDoc,
                "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[2]/"
                "x14:cfRule/xm:f[1]", "ISERROR(SEARCH($B$2,A2))");
    assertXPathContent(pDoc,
                "/x:worksheet/x:extLst/x:ext/x14:conditionalFormattings/x14:conditionalFormatting[2]/"
                "x14:cfRule/xm:f[2]", "$B$2");

    xDocSh->DoClose();
}

void ScExportTest::testTdf90104()
{
    ScDocShellRef xShell = loadDoc("tdf90104.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation/mc:AlternateContent"
                       "/mc:Choice/x12ac:list", "1,\"2,3\",4,\"5,6\"");
    assertXPathContent(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation/mc:AlternateContent"
                       "/mc:Fallback/x:formula1", "\"1,2,3,4,5,6\"");
}

void ScExportTest::testTdf111876()
 {
    // Document with relative path hyperlink

    ScDocShellRef xShell = loadDoc("tdf111876.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/_rels/sheet1.xml.rels", FORMAT_XLSX);
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

    ScDocShellRef xDocSh = saveAndReloadPassword(pShell, FORMAT_ODS);

    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rLoadedDoc = xDocSh->GetDocument();
    double aVal = rLoadedDoc.GetValue(0,0,0);
    ASSERT_DOUBLES_EQUAL(aVal, 1.0);

    xDocSh->DoClose();
}

void ScExportTest::testTdf134332()
{
    ScDocShellRef xShell = loadDoc("tdf134332.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocument& rDoc = xShell->GetDocument();

    ASSERT_DOUBLES_EQUAL(190.0, rDoc.GetValue(ScAddress(0,0,0)));

    ASSERT_DOUBLES_EQUAL(238.0, rDoc.GetValue(ScAddress(0,10144,0)));

    ScDocShellRef xDocSh = saveAndReloadPassword(xShell.get(), FORMAT_ODS);

    // Without the fixes in place, it would have failed here
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rLoadedDoc = xDocSh->GetDocument();
    ASSERT_DOUBLES_EQUAL(190.0, rLoadedDoc.GetValue(ScAddress(0,0,0)));

    ASSERT_DOUBLES_EQUAL(238.0, rLoadedDoc.GetValue(ScAddress(0,10144,0)));

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

void ScExportTest::testCondFormatExportCellIs()
{
    ScDocShellRef xShell = loadDoc("condFormat_cellis.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());
    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rDoc.GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = rDoc.GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);

    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    const ScCondFormatEntry* pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL( ScConditionMode::Equal,  pCondition->GetOperation());

    OUString aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL( OUString("$Sheet2.$A$2"), aStr );

    pEntry = pFormat->GetEntry(1);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL( ScConditionMode::Equal,  pCondition->GetOperation());

    aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL( OUString("$Sheet2.$A$1"), aStr );

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
    xmlDocUniquePtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
    assertXPath(pXmlDoc, "//office:spreadsheet[@table:structure-protected='true' and @table:protection-key='vbnhxyBKtPHCA1wB21zG1Oha8ZA=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");
    assertXPath(pXmlDoc, "//table:table[@table:protected='true' and @table:protection-key='vbnhxyBKtPHCA1wB21zG1Oha8ZA=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");

    xShell->DoClose();
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
    xmlDocUniquePtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
    assertXPath(pXmlDoc, "//office:spreadsheet[@table:structure-protected='true' and @table:protection-key='nLHas0RIwepGDaH4c2hpyIUvIS8=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");
    assertXPath(pXmlDoc, "//table:table[@table:protected='true' and @table:protection-key='nLHas0RIwepGDaH4c2hpyIUvIS8=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha1']");

    xShell->DoClose();
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
    xmlDocUniquePtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
    assertXPath(pXmlDoc, "//office:spreadsheet[@table:structure-protected='true' and @table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");
    assertXPath(pXmlDoc, "//table:table[@table:protected='true' and @table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");

    xShell->DoClose();
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
    xmlDocUniquePtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
    assertXPath(pXmlDoc, "//office:spreadsheet[@table:structure-protected='true' and @table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");
    assertXPath(pXmlDoc, "//table:table[@table:protected='true' and @table:protection-key='1tnJohagR2T0yF/v69hLPuumSTsj32CumW97nkKGuSQ=' and @table:protection-key-digest-algorithm='http://www.w3.org/2000/09/xmldsig#sha256']");

    xShell->DoClose();
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
    xmlDocUniquePtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
    assertXPath(pXmlDoc, "//office:spreadsheet[@table:structure-protected='true' and @table:protection-key='OX3WkEe79fv1PE+FUmfOLdwVoqI=' and @table:protection-key-digest-algorithm='http://docs.oasis-open.org/office/ns/table/legacy-hash-excel' and @loext:protection-key-digest-algorithm-2='http://www.w3.org/2000/09/xmldsig#sha1']");
    assertXPath(pXmlDoc, "//table:table[@table:protected='true' and @table:protection-key='OX3WkEe79fv1PE+FUmfOLdwVoqI=' and @table:protection-key-digest-algorithm='http://docs.oasis-open.org/office/ns/table/legacy-hash-excel' and @loext:protection-key-digest-algorithm-2='http://www.w3.org/2000/09/xmldsig#sha1']");

    xShell->DoClose();
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

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pComments
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/comments1.xml");
    CPPUNIT_ASSERT(pComments);

    assertXPathContent(pComments, "/x:comments/x:authors/x:author[1]", "BAKO");
    assertXPath(pComments, "/x:comments/x:authors/x:author", 1);

    assertXPathContent(pComments, "/x:comments/x:commentList/x:comment/x:text/x:r/x:t",
                       "Komentarz");

    xmlDocUniquePtr pVmlDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/vmlDrawing1.vml");
    CPPUNIT_ASSERT(pVmlDrawing);

    //assertXPath(pVmlDrawing, "/xml/v:shapetype", "coordsize", "21600,21600");
    assertXPath(pVmlDrawing, "/xml/v:shapetype", "spt", "202");
    assertXPath(pVmlDrawing, "/xml/v:shapetype/v:stroke", "joinstyle", "miter");
    const OUString sShapeTypeId = "#" + getXPath(pVmlDrawing, "/xml/v:shapetype", "id");

    assertXPath(pVmlDrawing, "/xml/v:shape", "type", sShapeTypeId);
    assertXPath(pVmlDrawing, "/xml/v:shape/v:shadow", "color", "black");
    assertXPath(pVmlDrawing, "/xml/v:shape/v:shadow", "obscured", "t");

    //tdf#117274 fix MSO interoperability with the secret VML shape type id
    assertXPath(pVmlDrawing, "/xml/v:shapetype", "id", "_x0000_t202");
    assertXPath(pVmlDrawing, "/xml/v:shape", "type", "#_x0000_t202");

    xShell->DoClose();
}

void ScExportTest::testCommentExportXLSX_2_XLSX()
{
    //tdf#117287 FILESAVE XLSX: Comments always disappear after opening the exported XLSX file with Excel
    ScDocShellRef xShell = loadDoc("tdf117287_comment.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());


    ScDocument& rDoc = xShell->GetDocument();
    ScAddress aPosC9(2, 8, 0);
    ScPostIt *pNote = rDoc.GetNote(aPosC9);

    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT(!pNote->IsCaptionShown());

    pNote->ShowCaption(aPosC9, true);

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pComments
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/comments1.xml");
    CPPUNIT_ASSERT(pComments);

    assertXPathContent(pComments, "/x:comments/x:commentList/x:comment/x:text/x:r/x:t",
                       "visible comment");

    xmlDocUniquePtr pVmlDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/vmlDrawing1.vml");
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPath(pVmlDrawing, "/xml/v:shape/x:ClientData/x:Visible", 0);

    xShell->DoClose();
}

#if HAVE_MORE_FONTS
void ScExportTest::testCustomColumnWidthExportXLSX()
{
    //tdf#100946 FILESAVE Excel on macOS ignored column widths in XLSX last saved by LO
    ScDocShellRef xShell = loadDoc("custom_column_width.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // tdf#124741: check that we export default width, otherwise the skipped columns would have
    // wrong width. Previously defaultColWidth attribute was missing
    double nDefWidth
        = getXPath(pSheet, "/x:worksheet/x:sheetFormatPr", "defaultColWidth").toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.53515625, nDefWidth, 0.01);

    // First column, has everything default (width in Calc: 1280), skipped

    // Second column, has custom width (width in Calc: 1225)
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "customWidth", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "min", "2");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[1]", "max", "2");

    // Third column, has everything default (width in Calc: 1280), skipped

    // Fourth column has custom width. Columns from 4 to 7 are hidden
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "customWidth", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "min", "4");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[2]", "max", "4");

    // 5th column has custom width. Columns from 4 to 7 are hidden
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "customWidth", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "min", "5");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[3]", "max", "5");

    // 6th and 7th columns have default width and they are hidden
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "customWidth", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "min", "6");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[4]", "max", "7");

    // 8th column has everything default - skipped

    // 9th column has custom width
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "customWidth", "true");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "min", "9");
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col[5]", "max", "9");

    // We expected that exactly 5 unique Nodes will be produced
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col", 5);

    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "hidden", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "outlineLevel", "0");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "collapsed", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "customFormat", "false");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "customHeight", "false");

    xShell->DoClose();
}
#endif

void ScExportTest::testXfDefaultValuesXLSX()
{
    //tdf#70565 FORMATTING: User Defined Custom Formatting is not applied during importing XLSX documents
    ScDocShellRef xShell = loadDoc("xf_default_values.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/styles.xml");
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

    xShell->DoClose();
}

namespace {

// TODO where to put this?
class Resetter
{
private:
    std::function<void ()> m_Func;

public:
    Resetter(std::function<void ()> const& rFunc)
        : m_Func(rFunc)
    {
    }
    ~Resetter()
    {
        try
        {
            m_Func();
        }
        catch (...) // has to be reliable
        {
            fprintf(stderr, "resetter failed with exception\n");
            abort();
        }
    }
};

} // namespace

static auto verifySpreadsheet13(char const*const pTestName, ScDocShellRef& pShell) -> void
{
    ScDocument const& rDoc(pShell->GetDocument());
    // OFFICE-2173 table:tab-color
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, Color(0xff3838), rDoc.GetTabBgColor(0));
    // OFFICE-3857 table:scale-to-X/table:scale-to-Y
    OUString styleName = rDoc.GetPageStyle(0);
    ScStyleSheetPool * pStylePool = rDoc.GetStyleSheetPool();
    SfxStyleSheetBase * pStyleSheet = pStylePool->Find(styleName, SfxStyleFamily::Page);
    CPPUNIT_ASSERT_MESSAGE(pTestName, pStyleSheet);

    SfxItemSet const& rSet = pStyleSheet->GetItemSet();
    ScPageScaleToItem const& rItem(rSet.Get(ATTR_PAGE_SCALETO));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, sal_uInt16(2), rItem.GetWidth());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, sal_uInt16(3), rItem.GetHeight());
}

void ScExportTest::testODF13()
{
    // import
    ScDocShellRef pShell = loadDoc("spreadsheet13e.", FORMAT_ODS);

    // check model
    verifySpreadsheet13("import", pShell);

    Resetter _([]() {
            std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
                comphelper::ConfigurationChanges::create());
            officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
            return pBatch->commit();
        });

    {
        // export ODF 1.3
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(10, pBatch);
        pBatch->commit();

        std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*pShell), FORMAT_ODS);

        // check XML
        xmlDocUniquePtr pContentXml = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
        assertXPath(pContentXml, "/office:document-content/office:automatic-styles/style:style/style:table-properties[@table:tab-color='#ff3838']");
        xmlDocUniquePtr pStylesXml = XPathHelper::parseExport(pXPathFile, m_xSFactory, "styles.xml");
        assertXPath(pStylesXml, "/office:document-styles/office:automatic-styles/style:page-layout/style:page-layout-properties[@style:scale-to-X='2']");
        assertXPath(pStylesXml, "/office:document-styles/office:automatic-styles/style:page-layout/style:page-layout-properties[@style:scale-to-Y='3']");

        // reload
        pShell = load(pXPathFile->GetURL(), "calc8", OUString(), OUString(), ODS_FORMAT_TYPE, SotClipboardFormatId::STARCALC_8);

        // check model
        verifySpreadsheet13("1.3 reload", pShell);
    }
    {
        // export ODF 1.2 Extended
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(9, pBatch);
        pBatch->commit();

        std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::saveAs(&(*pShell), FORMAT_ODS);
        pShell->DoClose();

        // check XML
        xmlDocUniquePtr pContentXml = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
        assertXPath(pContentXml, "/office:document-content/office:automatic-styles/style:style/style:table-properties[@tableooo:tab-color='#ff3838']");
        xmlDocUniquePtr pStylesXml = XPathHelper::parseExport(pXPathFile, m_xSFactory, "styles.xml");
        assertXPath(pStylesXml, "/office:document-styles/office:automatic-styles/style:page-layout/style:page-layout-properties[@loext:scale-to-X='2']");
        assertXPath(pStylesXml, "/office:document-styles/office:automatic-styles/style:page-layout/style:page-layout-properties[@loext:scale-to-Y='3']");

        // reload
        pShell = load(pXPathFile->GetURL(), "calc8", OUString(), OUString(), ODS_FORMAT_TYPE, SotClipboardFormatId::STARCALC_8);

        // check model
        verifySpreadsheet13("1.2 Extended reload", pShell);
    }
    {
        // export ODF 1.2
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(4, pBatch);
        pBatch->commit();

        std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::saveAs(&(*pShell), FORMAT_ODS);
        pShell->DoClose();

        // check XML
        xmlDocUniquePtr pContentXml = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");
        assertXPathNoAttribute(pContentXml, "/office:document-content/office:automatic-styles/style:style/style:table-properties", "tab-color");
        xmlDocUniquePtr pStylesXml = XPathHelper::parseExport(pXPathFile, m_xSFactory, "styles.xml");
        assertXPathNoAttribute(pStylesXml, "/office:document-styles/office:automatic-styles/style:page-layout[1]/style:page-layout-properties", "scale-to-X");
        assertXPathNoAttribute(pStylesXml, "/office:document-styles/office:automatic-styles/style:page-layout[1]/style:page-layout-properties", "scale-to-Y");

        // don't reload - no point
    }
}

void ScExportTest::testColumnWidthResaveXLSX()
{
    // tdf#91475 FILESAVE: Column width is not preserved in XLSX / after round trip.
    // Test if after resave .xlsx file, columns width is identical with previous one
    ScDocShellRef xShell = loadDoc("different-column-width-excel2010.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
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

    assertXPath(pSheet, "/x:worksheet/x:cols/x:col", 5);

    xShell->DoClose();
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
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
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

    // We expected that exactly 12 unique Nodes will be produced
    assertXPath(pSheet, "/x:worksheet/x:cols/x:col", 12);

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
    // Next rows are the same as the previous one but it needs to be preserved,
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

    xShell->DoClose();
}

void ScExportTest::testAllRowsHiddenXLSX()
{
    ScDocShellRef xOrigDocSh = loadDoc("tdf105840_allRowsHidden.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xOrigDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xOrigDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);
    assertXPath(pSheet, "/x:worksheet/x:sheetFormatPr", "zeroHeight", "true" );
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row", 0);

    xOrigDocSh->DoClose();
}

void ScExportTest::testHiddenEmptyRowsXLSX()
{
    //tdf#98106 FILESAVE: Hidden and empty rows became visible when export to .XLSX
    ScDocShellRef xShell = loadDoc("hidden-empty-rows.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:sheetFormatPr",  "zeroHeight", "false" );
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[2]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]", "hidden", "true");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]", "hidden", "false");

    xShell->DoClose();
}

void ScExportTest::testLandscapeOrientationXLSX()
{
    //tdf#48767 - Landscape page orientation is not loaded from .xlsx format with MS Excel, after export with Libre Office
    ScDocShellRef xShell = loadDoc("hidden-empty-rows.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // the usePrinterDefaults cannot be saved to allow opening sheets in Landscape mode via MS Excel
    assertXPathNoAttribute(pSheet, "/x:worksheet/x:pageSetup", "usePrinterDefaults");
    assertXPath(pSheet, "/x:worksheet/x:pageSetup", "orientation", "landscape");

    xShell->DoClose();
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
    static const TestParam::RowData DfltRowData[] =
    {
        { 0, 4, 0, 529, 0, false },
        { 5, 10, 0, 1058, 0, false },
        { 17, 20, 0, 1767, 0, false },
        // check last couple of row in document to ensure
        // they are 5.29mm ( effective default row xlsx height )
        { 1048573, 1048575, 0, 529, 0, false },
    };

    static const TestParam::RowData EmptyRepeatRowData[] =
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

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/workbook.xml", FORMAT_XLSX);
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
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(), [](const SfxPoolItem* p) {
                return p->Which() == EE_CHAR_WEIGHT &&
                    static_cast<const SvxWeightItem*>(p)->GetWeight() == WEIGHT_BOLD; });
        }

        static bool isItalic(const editeng::Section& rAttr)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(), [](const SfxPoolItem* p) {
                return p->Which() == EE_CHAR_ITALIC &&
                    static_cast<const SvxPostureItem*>(p)->GetPosture() == ITALIC_NORMAL; });
        }

        static bool isStrikeOut(const editeng::Section& rAttr)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(), [](const SfxPoolItem* p) {
                return p->Which() == EE_CHAR_STRIKEOUT &&
                    static_cast<const SvxCrossedOutItem*>(p)->GetStrikeout() == STRIKEOUT_SINGLE; });
        }

        static bool isOverline(const editeng::Section& rAttr, FontLineStyle eStyle)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(), [&eStyle](const SfxPoolItem* p) {
                return p->Which() == EE_CHAR_OVERLINE &&
                    static_cast<const SvxOverlineItem*>(p)->GetLineStyle() == eStyle; });
        }

        static bool isUnderline(const editeng::Section& rAttr, FontLineStyle eStyle)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(), [&eStyle](const SfxPoolItem* p) {
                return p->Which() == EE_CHAR_UNDERLINE &&
                    static_cast<const SvxUnderlineItem*>(p)->GetLineStyle() == eStyle; });
        }

        static bool isFont(const editeng::Section& rAttr, const OUString& rFontName)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(), [&rFontName](const SfxPoolItem* p) {
                return p->Which() == EE_CHAR_FONTINFO &&
                    static_cast<const SvxFontItem*>(p)->GetFamilyName() == rFontName; });
        }

        static bool isEscapement(const editeng::Section& rAttr, short nEsc, sal_uInt8 nRelSize)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(),
                [&nEsc, &nRelSize](const SfxPoolItem* p) {
                    if (p->Which() != EE_CHAR_ESCAPEMENT)
                        return false;
                    const SvxEscapementItem* pItem = static_cast<const SvxEscapementItem*>(p);
                    return ((pItem->GetEsc() == nEsc) && (pItem->GetProportionalHeight() == nRelSize));
                });
        }

        static bool isColor(const editeng::Section& rAttr, Color nColor)
        {
            return std::any_of(rAttr.maAttributes.begin(), rAttr.maAttributes.end(), [&nColor](const SfxPoolItem* p) {
                return p->Which() == EE_CHAR_COLOR &&
                    static_cast<const SvxColorItem*>(p)->GetValue() == nColor; });
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
            const editeng::Section* pAttr = aSecAttrs.data();
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
            const editeng::Section* pAttr = aSecAttrs.data();
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
            const editeng::Section* pAttr = aSecAttrs.data();
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
            const editeng::Section* pAttr = aSecAttrs.data();
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
            const editeng::Section* pAttr = aSecAttrs.data();
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
        pEE->SetTextCurrentDefaults("Bold and Italic");
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
        pEE->SetTextCurrentDefaults("One\nTwo\nThree");
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
        pEE->SetTextCurrentDefaults("\nTwo\nThree\n\nFive\n");
        rDoc3.SetEditText(ScAddress(1,4,0), pEE->CreateTextObject());
        pEditText = rDoc3.GetEditText(ScAddress(1,4,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B5 value.", aCheckFunc.checkB5(pEditText));

        // Insert a text with strikethrough in B6.
        pEE->Clear();
        pEE->SetTextCurrentDefaults("Strike Me");
        // Set the 'Strike' part strikethrough.
        setAttribute(*pEE, 0, 0, 6, EE_CHAR_STRIKEOUT);
        rDoc3.SetEditText(ScAddress(1,5,0), pEE->CreateTextObject());
        pEditText = rDoc3.GetEditText(ScAddress(1,5,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B6 value.", aCheckFunc.checkB6(pEditText));

        // Insert a text with different font segments in B7.
        pEE->Clear();
        pEE->SetTextCurrentDefaults("Font1 and Font2");
        setFont(*pEE, 0, 0, 5, "Courier");
        setFont(*pEE, 0, 10, 15, "Luxi Mono");
        rDoc3.SetEditText(ScAddress(1,6,0), pEE->CreateTextObject());
        pEditText = rDoc3.GetEditText(ScAddress(1,6,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B7 value.", aCheckFunc.checkB7(pEditText));

        // Insert a text with overline and underline in B8.
        pEE->Clear();
        pEE->SetTextCurrentDefaults("Over and Under");
        setAttribute(*pEE, 0, 0, 4, EE_CHAR_OVERLINE);
        setAttribute(*pEE, 0, 9, 14, EE_CHAR_UNDERLINE);
        rDoc3.SetEditText(ScAddress(1,7,0), pEE->CreateTextObject());
        pEditText = rDoc3.GetEditText(ScAddress(1,7,0));
        CPPUNIT_ASSERT_MESSAGE("Incorrect B8 value.", aCheckFunc.checkB8(pEditText));

        pEE->Clear();
        pEE->SetTextCurrentDefaults("Sub and Super");
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
        pEE->SetTextCurrentDefaults("BLUE AUTO");
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
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // make sure the only cell in this doc is assigned some formatting record
    OUString aCellFormat = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row/x:c", "s");
    CPPUNIT_ASSERT_MESSAGE("Cell format is missing", !aCellFormat.isEmpty());

    xmlDocUniquePtr pStyles = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/styles.xml");
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

            bIsMatrix = pCell->GetMatrixOrigin(rDoc, aMatOrigin);
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

    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pTempFile, m_xSFactory, "content.xml");
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
    static const struct
    {
        SCROW mnRow;
        SvxBorderLineStyle mnStyle;
        tools::Long mnWidth;
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
        tools::Long                      WidthTop, WidthBottom;
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

static OUString toString( const ScBigRange& rRange )
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
            static const CheckItem aChecks[] =
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
    pAction->GetDescription(aDesc, *pDoc);
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

            static const Color aXclColors[] =
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
            sc::TokenStringContext aCxt(rDoc, eGram);

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

            // C2:C7 should show 10,20,...,60.
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
                // B2:B7 should show A,B,...,F.
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
                // C2:C7 should show AA,BB,...,FF.
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
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/workbook.xml");
    CPPUNIT_ASSERT(pSheet);

    // there shouldn't be any defined names during export of FLOOR and CEILING functions to .xlsx
    assertXPath(pSheet, "/x:workbook/x:definedNames", 0);

    xShell->DoClose();
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
    xmlDocUniquePtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "customXml/item1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    xmlDocUniquePtr pRelsDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "customXml/_rels/item1.xml.rels");
    CPPUNIT_ASSERT(pRelsDoc);

    // Check there is a relation to itemProps1.xml.
    assertXPath(pRelsDoc, "/r:Relationships/r:Relationship", 1);
    assertXPath(pRelsDoc, "/r:Relationships/r:Relationship[@Id='rId1']", "Target", "itemProps1.xml");

    std::unique_ptr<SvStream> pStream = XPathHelper::parseExportStream(pXPathFile, m_xSFactory, "ddp/ddpfile.xen");
    CPPUNIT_ASSERT(pStream);

    xShell->DoClose();
}

#ifdef _WIN32
static sal_Unicode lcl_getWindowsDrive(const OUString& aURL)
{
    static const sal_Int32 nMinLen = strlen("file:///X:/");
    if (aURL.getLength() <= nMinLen)
        return 0;
    const OUString aUrlStart = aURL.copy(0, nMinLen);
    return (aUrlStart.startsWith("file:///") && aUrlStart.endsWith(":/")) ? aUrlStart[8] : 0;
}
#endif

void ScExportTest::testRelativePathsODS()
{
    ScDocShellRef xDocSh = loadDoc("fdo79305.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pTempFile = exportTo(xDocSh.get(), FORMAT_ODS);
    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pTempFile, m_xSFactory, "content.xml");
    CPPUNIT_ASSERT(pDoc);
    OUString aURL = getXPath(pDoc,
            "/office:document-content/office:body/office:spreadsheet/table:table/table:table-row[2]/table:table-cell[2]/text:p/text:a", "href");
#ifdef _WIN32
    // if the exported document is not on the same drive then the linked document,
    // there is no way to get a relative URL for the link, because ../X:/ is undefined.
    if (!aURL.startsWith(".."))
    {
        sal_Unicode aDocDrive = lcl_getWindowsDrive(pTempFile->GetURL());
        sal_Unicode aLinkDrive = lcl_getWindowsDrive(aURL);
        CPPUNIT_ASSERT_MESSAGE("document on the same drive but no relative link!",
                               aDocDrive != 0 && aLinkDrive != 0 && aDocDrive != aLinkDrive);
        return;
    }
#endif
    // make sure that the URL is relative
    CPPUNIT_ASSERT(aURL.startsWith(".."));

    xDocSh->DoClose();
}

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

        const OString sFailedMessage = OStringLiteral("Failed on filter: ") + aFilterNames[nFilter];
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

void ScExportTest::testSupBookVirtualPathXLS()
{
    ScDocShellRef xShell = loadDoc("external-ref.", FORMAT_XLS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    ScAddress aPos(0,0,0);
    ScTokenArray* pCode = getTokens(rDoc, aPos);
    if (!pCode)
        CppUnit::Asserter::fail("empty token array", CPPUNIT_SOURCELINE());

    OUString aFormula = toString(rDoc, aPos, *pCode, rDoc.GetGrammar());
#ifdef _WIN32
    aFormula = aFormula.copy(0, 9) + aFormula.copy(12); // strip drive letter, e.g. 'C:/'
#endif
    OUString aExpectedFormula = "'file:///home/timar/Documents/external.xls'#$Sheet1.A1";
    if (aFormula != aExpectedFormula)
    {
        CppUnit::Asserter::failNotEqual(to_std_string(aExpectedFormula),
            to_std_string(aFormula), CPPUNIT_SOURCELINE(), CppUnit::AdditionalMessage("Wrong SupBook VirtualPath URL"));
    }

    xDocSh->DoClose();
}

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
        const OString sFailedMessage = OStringLiteral("Failed on filter: ") + aFilterNames[nFilter];

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

        const OString sFailedMessage = OStringLiteral("Failed on filter: ") + aFilterNames[nFilter];
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
    ScRangeData* pRangeData = rDoc.GetRangeName()->findByUpperName(OUString("HTML_1"));
    ScSingleRefData* pRef = pRangeData->GetCode()->FirstToken()->GetSingleRef();
    // see tdf#119141 for the reason why this isn't Sheet1.HTML_1
    CPPUNIT_ASSERT_MESSAGE("HTML_1 is an absolute reference",!pRef->IsTabRel());

    xDocSh2->DoClose();
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

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/xdr:wsDr[1]/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:nvSpPr[1]/xdr:cNvPr[1]/a:hlinkClick[1]", 1);

    xDocSh->DoClose();
}

void ScExportTest::testFontSizeXLSX()
{
    ScDocShellRef xDocSh = loadDoc("fontSize.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    OUString fontSize = getXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr", "sz");
    // make sure that the font size is 18
    CPPUNIT_ASSERT_EQUAL(OUString("1800"), fontSize);

    xDocSh->DoClose();
}

void ScExportTest::testSheetCharacterKerningSpaceXLSX()
{
    ScDocShellRef xShell = loadDoc("textbox-CharKerningSpace.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
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

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
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

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
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

    xDocSh->DoClose();
}

void ScExportTest::testSheetRunParagraphPropertyXLSX()
{
    ScDocShellRef xShell = loadDoc("TextColor.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/sharedStrings.xml", FORMAT_XLSX);
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

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/sharedStrings.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:sst/x:si/x:t", "space", "preserve");
    xDocSh->DoClose();
}

void ScExportTest::testPreserveTextWhitespace2XLSX()
{
    ScDocShellRef xShell = loadDoc("preserve_space.", FORMAT_XLSX);
    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/sharedStrings.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:sst/x:si[1]/x:t", "space", "preserve");
    assertXPath(pDoc, "/x:sst/x:si[2]/x:r[1]/x:t", "space", "preserve");
    assertXPath(pDoc, "/x:sst/x:si[2]/x:r[2]/x:t", "space", "preserve");
    xDocSh->DoClose();
}

void ScExportTest::testHiddenShapeXLS()
{
    ScDocShellRef xDocSh = loadDoc("hiddenShape.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(rDoc.GetTableCount() > 0);
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);
    CPPUNIT_ASSERT_MESSAGE("Drawing object should not be visible.", !pObj->IsVisible());
    CPPUNIT_ASSERT_MESSAGE("Drawing object should not be printable.", !pObj->IsPrintable());
    xDocSh->DoClose();
}

void ScExportTest::testHiddenShapeXLSX()
{
    ScDocShellRef xDocSh = loadDoc("hiddenShape.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(rDoc.GetTableCount() > 0);
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);
    CPPUNIT_ASSERT_MESSAGE("Drawing object should not be visible.", !pObj->IsVisible());
    CPPUNIT_ASSERT_MESSAGE("Drawing object should not be printable.", !pObj->IsPrintable());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp[1]/xdr:nvSpPr/xdr:cNvPr", "hidden", "1");
    xDocSh->DoClose();
}

void ScExportTest::testShapeAutofitXLSX()
{
    ScDocShellRef xDocSh = loadDoc("testShapeAutofit.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/drawing1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    // TextAutoGrowHeight --> "Fit height to text" / "Resize shape to fit text" --> true
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:sp/xdr:txBody/a:bodyPr/a:spAutoFit", 1);
    // TextAutoGrowHeight --> "Fit height to text" / "Resize shape to fit text" --> false
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:sp/xdr:txBody/a:bodyPr/a:noAutofit", 1);

    xDocSh->DoClose();
}

void ScExportTest::testHyperlinkXLSX()
{
    ScDocShellRef xDocSh = loadDoc("hyperlink.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/_rels/drawing1.xml.rels", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/r:Relationships/r:Relationship", "Target", "#Sheet2!A1");

    xDocSh->DoClose();
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

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
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

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment", "readingOrder", "1");//LTR
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[3]/x:alignment", "readingOrder", "2");//RTL

    xDocSh->DoClose();
}

void ScExportTest::testTdf66668()
{
    // Would hang on exporting without the fix in place
    ScDocShellRef xDocSh = loadDoc("tdf66668.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoClose();
}

void ScExportTest::testTdf130108()
{
    ScDocShellRef xDocSh = loadDoc("tdf130108.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:b", "val", "1");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:i", "val", "0");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:color", "rgb", "FFFFFFFF");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:sz", "val", "10");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:fill/x:patternFill/x:bgColor", "rgb", "FFCC0000");

    xDocSh->DoClose();
}

void ScExportTest::testTdf76949()
{
    ScDocShellRef xDocSh = loadDoc("tdf76949.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pSheet = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pSheet);

    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c/x:f", "_xlfn.CHISQ.DIST(1,1,1)");

    xDocSh->DoClose();
}

void ScExportTest::testTdf55417()
{
    ScDocShellRef xDocSh = loadDoc("tdf55417.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[1]/x:alignment", 1);
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment", 1);

    xDocSh->DoClose();
}

void ScExportTest::testTdf129985()
{
    ScDocShellRef xDocSh = loadDoc("tdf129985.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode", "m/d/yyyy");

    xDocSh->DoClose();
}

void ScExportTest::testTdf73063()
{
    ScDocShellRef xDocSh = loadDoc("tdf73063.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode", "[$-1C1A]dddd\", \"d\". \"mmmm\\ yyyy;@");

    xDocSh->DoClose();
}

xmlDocUniquePtr ScExportTest::testTdf95640(const OUString& rFileName, sal_Int32 nSourceFormat,
                                           sal_Int32 nDestFormat)
{
    ScDocShellRef xShell = loadDoc(rFileName, nSourceFormat);
    CPPUNIT_ASSERT(xShell);

    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), nDestFormat);
    xShell->DoClose();

    return XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
}

void ScExportTest::testTdf95640_ods_to_xlsx()
{
    // Roundtripping sort options with user defined list to XLSX
    xmlDocUniquePtr pDoc = testTdf95640("tdf95640.", FORMAT_ODS, FORMAT_XLSX);

    assertXPath(pDoc, "//x:worksheet/x:autoFilter", "ref", "A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "ref", "A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "customList",
                "Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec");
}

void ScExportTest::testTdf95640_ods_to_xlsx_with_standard_list()
{
    // Roundtripping sort options with user defined list to XLSX
    xmlDocUniquePtr pDoc = testTdf95640("tdf95640_standard_list.", FORMAT_ODS, FORMAT_XLSX);

    assertXPath(pDoc, "//x:worksheet/x:autoFilter", "ref", "A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "ref", "A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "customList",
                "Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday");
}

void ScExportTest::testTdf95640_xlsx_to_xlsx()
{
    // XLSX Roundtripping sort options with custom sort list - note
    // that compared to ODS source documents above, here we _actually_
    // can use custom lists (beyond the global user defines), like
    // low, medium, high
    xmlDocUniquePtr pDoc = testTdf95640("tdf95640.", FORMAT_XLSX, FORMAT_XLSX);

    assertXPath(pDoc, "//x:worksheet/x:autoFilter", "ref", "A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "ref", "A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "customList",
                "Low,Medium,High");
}

void ScExportTest::testTdf88657ODS()
{
    ScDocShellRef xDocSh = loadDoc("tdf88657.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "styles.xml", FORMAT_ODS);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//number:fraction", "min-denominator-digits", "3");

    xDocSh->DoClose();
}

void ScExportTest::testConditionalFormatRangeListXLSX()
{
    ScDocShellRef xDocSh = loadDoc("conditionalformat_rangelist.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:conditionalFormatting", "sqref", "F4 F10");

    xDocSh->DoClose();
}

void ScExportTest::testConditionalFormatContainsTextXLSX()
{
    ScDocShellRef xDocSh = loadDoc("conditionalformat_containstext.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "//x:conditionalFormatting/x:cfRule/x:formula", "NOT(ISERROR(SEARCH(\"test\",A1)))");

    xDocSh->DoClose();
}

void ScExportTest::testConditionalFormatPriorityCheckXLSX()
{
    ScDocShellRef xDocSh = loadDoc("conditional_fmt_checkpriority.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    constexpr bool bHighPriorityExtensionA1 = true;  // Should A1's extension cfRule has higher priority than normal cfRule ?
    constexpr bool bHighPriorityExtensionA3 = false; // Should A3's extension cfRule has higher priority than normal cfRule ?

    size_t nA1NormalPriority = 0;
    size_t nA1ExtPriority = 0;
    size_t nA3NormalPriority = 0;
    size_t nA3ExtPriority = 0;

    for (size_t nIdx = 1; nIdx <= 2; ++nIdx)
    {
        OString aIdx = OString::number(nIdx);
        OUString aCellAddr = getXPath(pDoc, "//x:conditionalFormatting[" + aIdx + "]", "sqref");
        OUString aPriority = getXPath(pDoc, "//x:conditionalFormatting[" + aIdx + "]/x:cfRule", "priority");

        CPPUNIT_ASSERT_MESSAGE("conditionalFormatting sqref must be either A1 or A3", aCellAddr == "A1" || aCellAddr == "A3");

        if (aCellAddr == "A1")
            nA1NormalPriority = aPriority.toUInt32();
        else
            nA3NormalPriority = aPriority.toUInt32();

        aCellAddr = getXPathContent(pDoc, "//x:extLst/x:ext[1]/x14:conditionalFormattings/x14:conditionalFormatting[" + aIdx + "]/xm:sqref");
        aPriority = getXPath(pDoc, "//x:extLst/x:ext[1]/x14:conditionalFormattings/x14:conditionalFormatting[" + aIdx + "]/x14:cfRule", "priority");

        CPPUNIT_ASSERT_MESSAGE("x14:conditionalFormatting sqref must be either A1 or A3", aCellAddr == "A1" || aCellAddr == "A3");

        if (aCellAddr == "A1")
            nA1ExtPriority = aPriority.toUInt32();
        else
            nA3ExtPriority = aPriority.toUInt32();
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong priorities for A1", bHighPriorityExtensionA1, nA1ExtPriority < nA1NormalPriority);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong priorities for A3", bHighPriorityExtensionA3, nA3ExtPriority < nA3NormalPriority);

    xDocSh->DoClose();
}

void ScExportTest::testConditionalFormatOriginXLSX()
{
    ScDocShellRef xDocSh = loadDoc("conditional_fmt_origin.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    // tdf#124953 : The range-list is B3:C6 F1:G2, origin address in the formula should be B1, not B3.
    OUString aFormula = getXPathContent(pDoc, "//x:conditionalFormatting/x:cfRule/x:formula");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong origin address in formula", OUString("NOT(ISERROR(SEARCH(\"BAC\",B1)))"), aFormula);

    xDocSh->DoClose();
}

void ScExportTest::testTdf41722()
{
    ScDocShellRef xDocSh = loadDoc("tdf41722.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[1]", "operator", "containsText");
    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[2]", "operator", "containsText");
    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[3]", "operator", "containsText");

    xDocSh->DoClose();
}

void ScExportTest::testTdf113621()
{
    ScDocShellRef xDocSh = loadDoc("tdf113621.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:conditionalFormatting", "sqref", "A1:A1048576");

    xDocSh->DoClose();
}

void ScExportTest::testEscapeCharInNumberFormatXLSX()
{
    ScDocShellRef xDocSh = loadDoc("tdf81939.", FORMAT_XLSX);
    CPPUNIT_ASSERT( xDocSh.is() );
    xDocSh = saveAndReload( &(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT( xDocSh.is() );

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    const sal_Unicode cEuro (8364);  // € symbol
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode", "00\\ 00\\ 00\\ 00\\ 00");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode", "00\\.00\\.00\\.000\\.0");   // tdf#81939
    // "_-* #,##0\ _€_-;\-* #,##0\ _€_-;_-* "- "_€_-;_-@_-" // tdf#81222
    OUString rFormatStrExpected ( "_-* #,##0\\ _" + OUStringChar(cEuro) + "_-;\\-* #,##0\\ _" +
            OUStringChar(cEuro) + "_-;_-* \"- \"_" + OUStringChar(cEuro) + "_-;_-@_-" );
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[4]", "formatCode", rFormatStrExpected );
    // "_-* #,##0" €"_-;\-* #,##0" €"_-;_-* "- €"_-;_-@_-");
    rFormatStrExpected = "_-* #,##0\" " + OUStringChar(cEuro) + "\"_-;\\-* #,##0\" " +
            OUStringChar(cEuro) + "\"_-;_-* \"- " + OUStringChar(cEuro) + "\"_-;_-@_-";
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

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
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

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format lost exponent without sign during Excel export", OUString("0.00E0"), rFormatStr);

    xDocSh->DoClose();
}

void ScExportTest::testExtendedLCIDXLSX()
{
    ScDocShellRef xDocSh = loadDoc("tdf36038_ExtendedLCID.", FORMAT_ODS);
    CPPUNIT_ASSERT( xDocSh.is() );
    xDocSh = saveAndReload( &(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT( xDocSh.is() );

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    // Check export
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode", "[$-107041E]dd\\-mm\\-yyyy");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode", "[$-D07041E]dd\\-mm\\-yyyy");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[4]", "formatCode", "[$-1030411]dd\\-mm\\-ee");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[5]", "formatCode", "[$-1B030411]dd\\-mm\\-ee");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[6]", "formatCode", "[$-108040D]dd\\-mm\\-yyyy");
    //assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[7]", "formatCode", "[$-108040D]dd\\-mm\\-yyyy");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[7]", "formatCode", "[$-1060401]dd\\-mm\\-yyyy");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[8]", "formatCode", "[$-2060401]dd\\-mm\\-yyyy");

    // Check import
    ScDocument& rDoc = xDocSh->GetDocument();
    SvNumberFormatter* pNumFormatter = rDoc.GetFormatTable();
    sal_uInt32 nNumberFormat;
    const OUString aLang[5] = { "[$-41E]", "[$-411]", "[$-40D]", "[$-401]", "[$-500]" };
    const OUString aCalendar[5] = { "[~buddhist]DD-MM-YYYY", "[~gengou]DD-MM-EE", "[~jewish]DD-MM-YYYY", "[~hijri]DD-MM-YYYY", "[~dangi]YYYY/MM/DD" };
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

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "content.xml", FORMAT_ODS);
    CPPUNIT_ASSERT(pDoc);
    OUString aTargetFrameExport = getXPath(pDoc,
            "/office:document-content/office:body/office:spreadsheet/table:table/table:table-row[2]/table:table-cell[2]/text:p/text:a", "target-frame-name");
    CPPUNIT_ASSERT_EQUAL(OUString("_blank"), aTargetFrameExport);

    xDocSh->DoClose();
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

void ScExportTest::testKeepSettingsOfBlankRows()
{
    ScDocShellRef xDocSh = loadDoc("tdf41425.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // saved blank row with not default setting in A2
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row", 2);

    xDocSh->DoClose();
}

void ScExportTest::testTdf133595()
{
    ScDocShellRef xDocSh = loadDoc("tdf133595.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // without the fix in place, mc:AlternateContent would have been added to sheet1
    assertXPath(pSheet, "/x:worksheet/mc:AlternateContent", 0);

    xDocSh->DoClose();
}

void ScExportTest::testTdf134769()
{
    ScDocShellRef xDocSh = loadDoc("tdf134769.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // without the fix in place, the legacyDrawing would have been exported after the checkbox
    // and Excel would have claimed the document is corrupted
    // Use their ids to check the order
    assertXPath(pSheet, "/x:worksheet/x:drawing", "id", "rId2");
    assertXPath(pSheet, "/x:worksheet/x:legacyDrawing", "id", "rId3");
    assertXPath(pSheet, "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/mc:Choice/x:control", "id", "rId4");

    xDocSh->DoClose();
}

void ScExportTest::testTdf106181()
{
    ScDocShellRef xDocSh = loadDoc("tdf106181.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/mc:Choice/x:control", "name", "Check Box");
    assertXPath(pSheet, "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/mc:Choice/x:control/x:controlPr", "altText", "Check Box 1");

    xmlDocUniquePtr pDrawing = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(pDrawing, "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr", "name", "Check Box 1");
    assertXPath(pDrawing, "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr", "descr", "Check Box");
    assertXPath(pDrawing, "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr", "hidden", "0");

    xDocSh->DoClose();
}

void ScExportTest::testTdf105272()
{
    ScDocShellRef xDocSh = loadDoc("tdf105272.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xDocSh = saveAndReload(xDocSh.get(), FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();
    //without the fix in place,it would fail
    //Expected: Table1[[#This Row],[Total]]/Table1[[#This Row],['# Athletes]]
    //Actual  : table1[[#this row],[total]]/table1[[#this row],['# athletes]]

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(7, 3, 0),
                         "Table1[[#This Row],[Total]]/Table1[[#This Row],['# Athletes]]",
                         "Wrong formula");
}

void ScExportTest::testTdf118990()
{
    ScDocShellRef xDocSh = loadDoc("tdf118990.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xDocSh = saveAndReload(xDocSh.get(), FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();

    // TODO: also test A1, which contains a UNC reference to \\localhost\share\lookupsource.xlsx,
    // but currently looses "localhost" part when normalized in INetURLObject, becoming
    // file:///share/lookupsource.xlsx - which is incorrect, since it points to local filesystem
    // and not to Windows network share.

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(0, 1, 0),
                         "VLOOKUP(B1,'file://192.168.1.1/share/lookupsource.xlsx'#$Sheet1.A1:B5,2)",
                         "Wrong Windows share (using host IP) URL in A2");

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(0, 2, 0),
                         "VLOOKUP(B1,'file://NETWORKHOST/share/lookupsource.xlsx'#$Sheet1.A1:B5,2)",
                         "Wrong Windows share (using hostname) URL in A3");

    xDocSh->DoClose();
}

void ScExportTest::testTdf121612()
{
    ScDocShellRef xDocSh = loadDoc("tdf121612.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    xDocSh = saveAndReload(xDocSh.get(), FORMAT_XLSX);

    ScDocument& rDoc = xDocSh->GetDocument();

    // There should be a pivot table
    CPPUNIT_ASSERT(rDoc.HasPivotTable());

    // DP collection is not lost after export and has one entry
    ScDPCollection* pDPColl = rDoc.GetDPCollection();
    CPPUNIT_ASSERT(pDPColl);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDPColl->GetCount());

    xDocSh->DoClose();
}

void ScExportTest::testTdf112936()
{
    ScDocShellRef xDocSh = loadDoc("tdf112936.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/pivotCache/pivotCacheDefinition1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:pivotCacheDefinition", "recordCount", "4");
    assertXPath(pDoc, "//x:pivotCacheDefinition", "createdVersion", "3");

    xDocSh->DoClose();
}

void ScExportTest::testXltxExport()
{
    // Create new document
    ScDocShell* pShell = new ScDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    pShell->DoInitNew();

    // Export as template and check content type
    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *pShell, m_xSFactory, "[Content_Types].xml", FORMAT_XLTX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/ContentType:Types/ContentType:Override[@PartName='/xl/workbook.xml']",
        "ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml");
}

void ScExportTest::testPivotCacheAfterExportXLSX()
{
    ScDocShellRef xDocSh = loadDoc("numgroup_example.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    // export only
    std::shared_ptr<utl::TempFile> pTemp = saveAs(xDocSh.get(), FORMAT_XLSX);

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(rDoc.HasPivotTable());

    // Two pivot tables
    ScDPCollection* pDPColl = rDoc.GetDPCollection();
    CPPUNIT_ASSERT(pDPColl);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDPColl->GetCount());

    // One cache
    ScDPCollection::SheetCaches& rSheetCaches = pDPColl->GetSheetCaches();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rSheetCaches.size());
    const ScDPCache* pCache = rSheetCaches.getExistingCache(ScRange(0, 0, 0, 3, 30, 0));
    CPPUNIT_ASSERT_MESSAGE("Pivot cache is expected for A1:D31 on the first sheet.", pCache);

    // See if XLSX export didn't damage group info of the 1st pivot table
    const ScDPNumGroupInfo* pInfo = pCache->GetNumGroupInfo(1);
    CPPUNIT_ASSERT_MESSAGE("No number group info :(", pInfo);

    xDocSh->DoClose();
}

void ScExportTest::testTdf114969XLSX()
{
    ScDocShellRef xDocSh = loadDoc("sheet_name_with_dots.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[1]", "location", "'1.1.1.1'!C1");
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[2]", "location", "'1.1.1.1'!C2");

    xDocSh->DoClose();
}

void ScExportTest::testTdf115192XLSX()
{
    ScDocShellRef xDocSh = loadDoc("test_115192.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/drawings/_rels/drawing1.xml.rels", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/r:Relationships/r:Relationship[@Id='rId1']", "TargetMode", "External");
    assertXPathNoAttribute(pDoc, "/r:Relationships/r:Relationship[@Id='rId2']", "TargetMode");
    assertXPath(pDoc, "/r:Relationships/r:Relationship[@Id='rId3']", "TargetMode", "External");

    xDocSh->DoClose();
}

void ScExportTest::testTdf91634XLSX()
{
    ScDocShellRef xDocSh = loadDoc("image_hyperlink.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:pic/xdr:nvPicPr/xdr:cNvPr/a:hlinkClick", 1);

    xmlDocUniquePtr pXmlRels = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship[@Id='rId1']", "Target", "https://www.google.com/");
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship[@Id='rId1']", "TargetMode", "External");

    xDocSh->DoClose();
}

void ScExportTest::testValidationCopyPaste()
{
    ScDocShellRef xDocSh = loadDoc("validation-copypaste.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rSrcDoc = xDocSh->GetDocument();

    // Copy B1 from src doc to clip
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(1, 0, 1);
    ScClipParam aClipParam(aSrcRange, false);
    ScMarkData aMark(rSrcDoc.GetSheetLimits());
    aMark.SetMarkArea(aSrcRange);
    rSrcDoc.CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Create second document, paste B1 from clip
    ScDocShell* pShell2
        = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT | SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS
                         | SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    pShell2->DoInitNew();
    ScDocument& rDestDoc = pShell2->GetDocument();
    ScRange aDstRange(1, 0, 0);
    ScMarkData aMark2(rDestDoc.GetSheetLimits());
    aMark2.SetMarkArea(aDstRange);
    rDestDoc.CopyFromClip(aDstRange, aMark2, InsertDeleteFlags::ALL, nullptr, &aClipDoc);

    // save as XLSX
    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*pShell2), FORMAT_XLSX);

    // check validation
    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPathContent(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation/x:formula1", "#REF!");
}

void ScExportTest::testTdf115159()
{
    ScDocShellRef xShell = loadDoc("tdf115159.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());
    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xShell->DoClose();

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/workbook.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName", 1);

    xDocSh->DoClose();
}

void ScExportTest::testTdf112567()
{
    // Set the system locale to Hungarian (a language with different range separator)
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString("hu-HU");
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    ScDocShellRef xShell = loadDoc("tdf112567.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());
    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xShell->DoClose();

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/workbook.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName", 1);

    xDocSh->DoClose();
}

void ScExportTest::testTdf112567b()
{
    // Set the system locale to Hungarian (a language with different range separator)
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString("hu-HU");
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    ScDocShellRef xShell = loadDoc("tdf112567.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());
    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xShell->DoClose();

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/workbook.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName", 1);

    //and it contains "," instead of ";"
    assertXPathContent(pDoc, "/x:workbook/x:definedNames/x:definedName[1]", "Sheet1!$A:$A,Sheet1!$1:$1");

    xDocSh->DoClose();
}

void ScExportTest::testTdf123645XLSX()
{
    ScDocShellRef xDocSh = loadDoc("chart_hyperlink.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:graphicFrame/xdr:nvGraphicFramePr/xdr:cNvPr/a:hlinkClick", 1);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:graphicFrame/xdr:nvGraphicFramePr/xdr:cNvPr/a:hlinkClick", 1);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:graphicFrame/xdr:nvGraphicFramePr/xdr:cNvPr/a:hlinkClick", 1);

    xmlDocUniquePtr pXmlRels = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship[@Id='rId1']", "TargetMode", "External");
    assertXPathNoAttribute(pXmlRels, "/r:Relationships/r:Relationship[@Id='rId3']", "TargetMode");
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship[@Id='rId5']", "TargetMode", "External");
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship[@Id='rId1']", "Target", "file:///C:/TEMP/test.xlsx");
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship[@Id='rId3']", "Target", "#Sheet2!A1");
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship[@Id='rId5']", "Target", "https://bugs.documentfoundation.org/show_bug.cgi?id=123645");

    xDocSh->DoClose();
}

void ScExportTest::testTdf125173XLSX()
{
    ScDocShellRef xDocSh = loadDoc("text_box_hyperlink.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr/a:hlinkClick", 1);

    xmlDocUniquePtr pXmlRels = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship[@Id='rId1']", "Target", "http://www.google.com/");
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship[@Id='rId1']", "TargetMode", "External");

    xDocSh->DoClose();
}

void ScExportTest::testTdf79972XLSX()
{
    ScDocShellRef xDocSh = loadDoc("tdf79972.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink", "ref", "A1");

    xmlDocUniquePtr pXmlRels = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/_rels/sheet1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship", "Target", "https://bugs.documentfoundation.org/show_bug.cgi?id=79972");
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship", "TargetMode", "External");

    xDocSh->DoClose();
}

void ScExportTest::testTdf126024XLSX()
{
    ScDocShellRef xDocSh = loadDoc("hyperlink_formula.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink", "ref", "A2");

    xmlDocUniquePtr pXmlRels = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/_rels/sheet1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship", "Target", "https://bugs.documentfoundation.org/");
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship", "TargetMode", "External");

    xDocSh->DoClose();
}

void ScExportTest::testTdf126177XLSX()
{
    ScDocShellRef xDocSh = loadDoc("hyperlink_export.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink", "location", "Munka1!A5");

    xmlDocUniquePtr pXmlRels = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/_rels/sheet1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    OUString aTarget = getXPath(pXmlRels, "/r:Relationships/r:Relationship", "Target");
    CPPUNIT_ASSERT(aTarget.endsWith("test.xlsx"));
    assertXPath(pXmlRels, "/r:Relationships/r:Relationship", "TargetMode", "External");

    xDocSh->DoClose();
}

void ScExportTest::testCommentTextVAlignment()
{
    // Testing comment text alignments.
    ScDocShellRef xShell = loadDoc("CommentTextVAlign.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pVmlDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/vmlDrawing1.vml");
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/xml/v:shape/xx:ClientData/xx:TextVAlign", "Center");

    xShell->DoClose();
}

void ScExportTest::testCommentTextHAlignment()
{
    // Testing comment text alignments.
    ScDocShellRef xShell = loadDoc("CommentTextHAlign.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pVmlDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/vmlDrawing1.vml");
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/xml/v:shape/xx:ClientData/xx:TextHAlign", "Center");

    xShell->DoClose();
}

void ScExportTest::testRotatedImageODS()
{
    // Error was, that the length values in shapes were not
    // written in the given unit into the file.
    css::uno::Reference<css::sheet::XGlobalSheetSettings> xGlobalSheetSettings
        = css::sheet::GlobalSheetSettings::create(comphelper::getProcessComponentContext());
    xGlobalSheetSettings->setMetric(static_cast<sal_Int16>(FieldUnit::MM));

    ScDocShellRef xDocSh = loadDoc("tdf103092_RotatedImage.", FORMAT_ODS, true);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pTemp = saveAs(xDocSh.get(), FORMAT_ODS);
    CPPUNIT_ASSERT(pTemp);
    xmlDocUniquePtr pXmlDoc = XPathHelper::parseExport(pTemp, m_xSFactory, "content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    const OUString sTransform = getXPath(
        pXmlDoc,
        "/office:document-content/office:body/office:spreadsheet/"
            "table:table/table:shapes/draw:frame",
        "transform");
    // Attribute transform has the structure skew (...) rotate (...) translate (x y)
    // parts are separated by blank
    OUString sTranslate(sTransform.copy(sTransform.lastIndexOf('(')));
    sTranslate = sTranslate.copy(1, sTranslate.getLength()-2); // remove '(' and ')'
    const OUString sX(sTranslate.getToken(0, ' '));
    const OUString sY(sTranslate.getToken(1, ' '));
    CPPUNIT_ASSERT(sX.endsWith("mm") && sY.endsWith("mm"));

    xDocSh->DoClose();
}

void ScExportTest::testTdf128976()
{
    ScDocShellRef xShell = loadDoc("tdf128976.", FORMAT_XLS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLS);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Trying to save very small fractional default column width to XLS (where only integer values
    // between 0 and 255 are allowed as default) resulted in negative (-1) value after correction,
    // and was written as 65535 (invalid default width). As the result, all columns had large width
    // when reopened: 28415 (and Excel warned about invalid format).
    const sal_uInt16 nColumn0Width = rDoc.GetColWidth(SCCOL(0), SCTAB(0), false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(45), nColumn0Width);

    xDocSh->DoClose();
}

void ScExportTest::testTdf120502()
{
    // Create an empty worksheet; resize last column on its first sheet; export to XLSX, and check
    // that the last exported column number is correct
    css::uno::Reference<css::frame::XDesktop2> xDesktop
        = css::frame::Desktop::create(comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop);

    css::uno::Sequence<css::beans::PropertyValue> args(1);
    args[0].Name = "Hidden";
    args[0].Value <<= true;

    css::uno::Reference<css::lang::XComponent> xComponent
        = xDesktop->loadComponentFromURL("private:factory/scalc", "_blank", 0, args);
    CPPUNIT_ASSERT(xComponent);

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShellRef xShell = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xShell);

    ScDocument& rDoc = xShell->GetDocument();
    const SCCOL nMaxCol = rDoc.MaxCol(); // 0-based

    const auto nOldWidth = rDoc.GetColWidth(nMaxCol, 0);
    rDoc.SetColWidth(nMaxCol, 0, nOldWidth + 100);

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xShell->DoClose();
    xmlDocUniquePtr pSheet1
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet1);

    // This was 1025 when nMaxCol+1 was 1024
    assertXPath(pSheet1, "/x:worksheet/x:cols/x:col", "max", OUString::number(nMaxCol + 1));
}

void ScExportTest::testTdf131372()
{
    ScDocShellRef xShell = loadDoc("tdf131372.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell);

    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c[1]/x:f", "NA()");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c[2]/x:f", "#N/A");

    xShell->DoClose();

}
void ScExportTest::testTdf81470()
{
    ScDocShellRef xShell = loadDoc("tdf81470.", FORMAT_XLS);
    CPPUNIT_ASSERT(xShell);

    //without the fix in place, it would have crashed at export time
    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    //also check revisions are exported
    xmlDocUniquePtr pHeaders = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/revisions/revisionHeaders.xml");
    CPPUNIT_ASSERT(pHeaders);

    assertXPath(pHeaders, "/x:headers/x:header[1]", "dateTime", "2014-07-11T13:46:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[1]", "userName", "Kohei Yoshida");
    assertXPath(pHeaders, "/x:headers/x:header[2]", "dateTime", "2014-07-11T18:38:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[2]", "userName", "Kohei Yoshida");
    assertXPath(pHeaders, "/x:headers/x:header[3]", "dateTime", "2014-07-11T18:43:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[3]", "userName", "Kohei Yoshida");

    xShell->DoClose();
}

void ScExportTest::testTdf122331()
{
    ScDocShellRef xShell = loadDoc("tdf122331.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell);

    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pSheet = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:sheetPr", "filterMode", "true");
    assertXPath(pSheet, "/x:worksheet/x:autoFilter", "ref", "A1:B761");
    assertXPath(pSheet, "/x:worksheet/x:autoFilter/x:filterColumn", "colId", "1");

    xShell->DoClose();
}

void ScExportTest::testTdf83779()
{
    // Roundtripping TRUE/FALSE constants (not functions) must convert them to functions
    ScDocShellRef xShell = loadDoc("tdf83779.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell);

    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pVmlDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/x:worksheet/x:sheetData/x:row[1]/x:c/x:f", "FALSE()");
    assertXPathContent(pVmlDrawing, "/x:worksheet/x:sheetData/x:row[2]/x:c/x:f", "TRUE()");

    xShell->DoClose();
}

void ScExportTest::testTdf121716_ExportEvenHeaderFooterXLSX()
{
    // Header and footer on even pages should be exported properly
    // If there are separate odd/even header, but only 1 footer for all pages (this is possible only in LibreOffice)
    //  then the footer will be duplicated to have the same footer separately for even/odd pages

    ScDocShellRef xShell = loadDoc("tdf121716_EvenHeaderFooter.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:headerFooter", "differentOddEven", "true");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader", "&Lodd/right&Cpage&Rheader");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter", "&Lboth&C&12page&Rfooter");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenHeader", "&Lpage&Cheader&Reven/left");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenFooter", "&Lboth&C&12page&Rfooter");

    pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet2.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:headerFooter", "differentOddEven", "true");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader", "&Coddh");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter", "&Coddf");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenHeader", "&Cevenh");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenFooter", "&Levenf");

    xDocSh->DoClose();
}

void ScExportTest::testTdf134459_HeaderFooterColorXLSX()
{
    // Colors in header and footer should be exported, and imported properly
    ScDocShellRef xShell = loadDoc("tdf134459_HeaderFooterColor.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader", "&L&Kc06040l&C&K4c3789c&Rr");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter", "&Ll&C&K64cf5fc&R&Kcd15aar");

    xDocSh->DoClose();
}

void ScExportTest::testTdf134817_HeaderFooterTextWith2SectionXLSX()
{
    // Header/footer text with multiple selection should be exported, and imported properly
    ScDocShellRef xShell = loadDoc("tdf134817_HeaderFooterTextWith2Section.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader", "&L&\"Abadi,Regular\"&11aaa&\"Bembo,Regular\"&20bbb");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter", "&R&\"Cambria,Regular\"&14camb&\"Dante,Regular\"&18dant");

    xDocSh->DoClose();
}

void ScExportTest::testTdf121718_UseFirstPageNumberXLSX()
{
    // If "First page number" is not checked then useFirstPageNumb, and firstPageNumber should not be exported.
    ScDocShellRef xShell = loadDoc("tdf121718_UseFirstPageNumber.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:pageSetup", "useFirstPageNumber", "true");
    assertXPath(pDoc, "/x:worksheet/x:pageSetup", "firstPageNumber", "10");

    pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet2.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPathNoAttribute(pDoc, "/x:worksheet/x:pageSetup", "useFirstPageNumber");
    assertXPathNoAttribute(pDoc, "/x:worksheet/x:pageSetup", "firstPageNumber");

    xDocSh->DoClose();
}

void ScExportTest::testHeaderFontStyleXLSX()
{
    ScDocShellRef xShell = loadDoc("tdf134826.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocument& rDoc = xShell->GetDocument();
    SfxStyleSheetBase* pStyleSheet = rDoc.GetStyleSheetPool()->Find(rDoc.GetPageStyle(0), SfxStyleFamily::Page);
    const SfxItemSet& rItemSet = pStyleSheet->GetItemSet();
    const ScPageHFItem& rHFItem = rItemSet.Get(ATTR_PAGE_HEADERRIGHT);
    const EditTextObject* pTextObj = rHFItem.GetLeftArea();

    std::vector<EECharAttrib> rLst;

    // first line is bold.
    pTextObj->GetCharAttribs(0, rLst);
    bool bHasBold = std::any_of(rLst.begin(), rLst.end(), [](const EECharAttrib& rAttrib) {
        return rAttrib.pAttr->Which() == EE_CHAR_WEIGHT &&
            static_cast<const SvxWeightItem&>(*rAttrib.pAttr).GetWeight() == WEIGHT_BOLD; });
    CPPUNIT_ASSERT_MESSAGE("First line should be bold.", bHasBold);

    // second line is italic.
    pTextObj->GetCharAttribs(1, rLst);
    bool bHasItalic = std::any_of(rLst.begin(), rLst.end(), [](const EECharAttrib& rAttrib) {
        return rAttrib.pAttr->Which() == EE_CHAR_ITALIC &&
            static_cast<const SvxPostureItem&>(*rAttrib.pAttr).GetPosture() == ITALIC_NORMAL; });
    CPPUNIT_ASSERT_MESSAGE("Second line should be italic.", bHasItalic);

    xShell->DoClose();
}

void ScExportTest::testTdf135828_Shape_Rect()
{
    // tdf#135828 Check that the width and the height of rectangle of the shape is correct.
    // tdf#123613 Check the positioning, and allow massive rounding errors because of the back and
    // forth conversion between emu and hmm.
    ScDocShellRef xShell = loadDoc("tdf135828_Shape_Rect.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDrawing = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    double nXPosOfTopleft = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:off", "x" ).toDouble();
    double nYPosOfTopleft = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:off", "y" ).toDouble();
    double nWidth         = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:ext", "cx").toDouble();
    double nHeight        = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:ext", "cy").toDouble();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(  854640, nXPosOfTopleft, 10000);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -570600, nYPosOfTopleft, 10000);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  294840,         nWidth, 10000);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 1988280,        nHeight, 10000);
}

void ScExportTest::testTdf123353()
{
    ScDocShellRef xShell = loadDoc("tdf123353.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:autoFilter/x:filterColumn/x:filters", "blank", "1");

    xShell->DoClose();
}

void ScExportTest::testTdf133688_precedents()
{
    // tdf#133688 Check that we do not export detective shapes.
    ScDocShellRef xShell = loadDoc("tdf133688_dont_save_precedents_to_xlsx.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    // We do not export any shapes.
    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor[1]", 0);
}

void ScExportTest::testTdf91251_missingOverflowRoundtrip()
{
    // tdf#91251 check whether textBox overflow property (horzOverflow and vertOverflow) is
    // getting preserved after roundtrip
    ScDocShellRef xShell = loadDoc("tdf91251_missingOverflowRoundtrip.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDrawing = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr", "horzOverflow", "clip");
    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr", "horzOverflow", "clip");
}

void ScExportTest::testTdf137000_handle_upright()
{
    // tdf#106197 When exporting the "upright" attribute, we must set
    // TextPreRotateAngle to 0.
    // (Upright is an xml attribute of xdr:txBody/a:bodyPr. It is set when
    // in a textbox menu we choose: do not rotate this element.)
    ScDocShellRef xShell = loadDoc("tdf137000_export_upright.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPathNoAttribute(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr",
                           "rot");
}

void ScExportTest::testTdf126305_DataValidatyErrorAlert()
{
    ScDocShellRef xShell = loadDoc("tdf126305.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[1]", "errorStyle", "stop");
    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[2]", "errorStyle", "warning");
    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[3]", "errorStyle", "information");

    xDocSh->DoClose();
}

void ScExportTest::testTdf76047_externalLink()
{
    ScDocShellRef pShell = loadDoc("tdf76047_externalLink.", FORMAT_XLSX);
    CPPUNIT_ASSERT(pShell.is());

    // load data from external links. (tdf76047_externalLinkSource.ods)
    // that file has to be in the same directory as tdf76047_externalLink.xlsx
    pShell->ReloadAllLinks();
    ScDocument& rDoc = pShell->GetDocument();

    // compare the data loaded from external links with the expected result stored in the test file
    for (int nCol = 1; nCol <= 5; nCol++)
    {
        for (int nRow = 3; nRow <= 5; nRow++)
        {
            OUString aStr1 = rDoc.GetString(ScAddress(nCol, nRow, 0));
            OUString aStr2 = rDoc.GetString(ScAddress(nCol, nRow + 5, 0));
            OUString aStr3 = rDoc.GetString(ScAddress(nCol, nRow + 11, 0));

            CPPUNIT_ASSERT_EQUAL(aStr1, aStr3);
            CPPUNIT_ASSERT_EQUAL(aStr2, aStr3);
        }
    }
}

void ScExportTest::testTdf87973_externalLinkSkipUnuseds()
{
    ScDocShellRef pShell = loadDoc("tdf87973_externalLinkSkipUnuseds.", FORMAT_ODS);
    CPPUNIT_ASSERT(pShell.is());

    // try to load data from external link: tdf132105_external.ods
    // that file has to be in the same directory as tdf87973_externalLinkSkipUnuseds.ods
    pShell->ReloadAllLinks();
    ScDocument& rDoc = pShell->GetDocument();

    // change external link to: 87973_externalSource.ods
    OUString aFormula, aFormula2;
    rDoc.GetFormula(3, 1, 0, aFormula);
    auto nIdxOfFilename = aFormula.indexOf("tdf132105_external.ods");
    aFormula = aFormula.replaceAt(nIdxOfFilename, 22, "87973_externalSource.ods");
    auto nIdxOfFile = aFormula.indexOf("file");

    // saveAndReload save the file to a temporary directory
    // the link must be changed to point to that directory
    utl::TempFile aTempFile;
    auto aTempFilename = aTempFile.GetURL();
    auto nIdxOfTmpFile = aTempFilename.lastIndexOf('/');
    aTempFilename = aTempFilename.copy(0, nIdxOfTmpFile + 1);

    aFormula = aFormula.replaceAt(nIdxOfFile, nIdxOfFilename - nIdxOfFile, aTempFilename);
    rDoc.SetFormula(ScAddress(3, 1, 0), aFormula, formula::FormulaGrammar::GRAM_NATIVE_UI);

    // tdf#138832: test the same thing with singleref link
    rDoc.GetFormula(3, 2, 0, aFormula);
    nIdxOfFilename = aFormula.indexOf("tdf132105_external.ods");
    aFormula = aFormula.replaceAt(nIdxOfFilename, 22, "87973_externalSource.ods");
    nIdxOfFile = aFormula.indexOf("file");

    aFormula = aFormula.replaceAt(nIdxOfFile, nIdxOfFilename - nIdxOfFile, aTempFilename);
    rDoc.SetFormula(ScAddress(3, 2, 0), aFormula, formula::FormulaGrammar::GRAM_NATIVE_UI);

    // save and load back
    ScDocShellRef pDocSh = saveAndReload(&(*pShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(pDocSh.is());

    // check if the the new filename is present in the link (and not replaced by '[2]')
    ScDocument& rDoc2 = pDocSh->GetDocument();
    rDoc2.GetFormula(3, 1, 0, aFormula2);
    CPPUNIT_ASSERT(aFormula2.indexOf("tdf132105_external.ods") < 0);
    CPPUNIT_ASSERT(aFormula2.indexOf("87973_externalSource.ods") >= 0);
    rDoc2.GetFormula(3, 2, 0, aFormula2);
    CPPUNIT_ASSERT(aFormula2.indexOf("tdf132105_external.ods") < 0);
    CPPUNIT_ASSERT(aFormula2.indexOf("87973_externalSource.ods") >= 0);

    pDocSh->DoClose();
}

void ScExportTest::testTdf51022_lostPrintRange()
{
    ScDocShellRef pShell = loadDoc(u"tdf87973_externalLinkSkipUnuseds.", FORMAT_ODS);
    CPPUNIT_ASSERT(pShell.is());

    pShell->ReloadAllLinks();
    ScDocument& rDoc = pShell->GetDocument();

    //Add print ranges
    ScRange aRange1(1, 2, 0, 3, 4, 0);
    ScRange aRange2(1, 6, 0, 3, 7, 0);
    rDoc.AddPrintRange(0, aRange1);
    rDoc.AddPrintRange(0, aRange2);

    // save and load back
    ScDocShellRef pDocSh = saveAndReload(&(*pShell), FORMAT_ODS);
    CPPUNIT_ASSERT(pDocSh.is());

    // check if the same print ranges are present
    ScDocument& rDoc2 = pDocSh->GetDocument();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), rDoc2.GetPrintRangeCount(0));
    CPPUNIT_ASSERT_EQUAL(aRange1, *rDoc2.GetPrintRange(0, 0));
    CPPUNIT_ASSERT_EQUAL(aRange2, *rDoc2.GetPrintRange(0, 1));

    pDocSh->DoClose();
}

void ScExportTest::testTdf138741_externalLinkSkipUnusedsCrash()
{
    ScDocShellRef xShell = loadDoc("tdf138741_externalLinkSkipUnusedsCrash.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell);

    //without the fix in place, it would have crashed at export time
    ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xShell->DoClose();
}

void ScExportTest::testTdf138824_linkToParentDirectory()
{
    ScDocShellRef xShell = loadDoc("childDir/tdf138824_linkToParentDirectory.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocument& rDoc = xShell->GetDocument();

    // saveAndReload save the file to a temporary directory
    // the link must be changed to point to that parent directory
    utl::TempFile aTempFile;
    auto aTempFilename = aTempFile.GetURL();
    auto nIdxOfTmpFile = aTempFilename.lastIndexOf('/');
    nIdxOfTmpFile = aTempFilename.lastIndexOf('/', nIdxOfTmpFile);
    aTempFilename = aTempFilename.copy(0, nIdxOfTmpFile + 1);

    // change external link to tmp directory
    OUString aFormula;
    rDoc.GetFormula(3, 1, 0, aFormula);
    auto nIdxOfFilename = aFormula.indexOf("tdf138824_externalSource.ods");
    auto nIdxOfFile = aFormula.indexOf("file");

    aFormula = aFormula.replaceAt(nIdxOfFile, nIdxOfFilename - nIdxOfFile, aTempFilename);
    rDoc.SetFormula(ScAddress(3, 1, 0), aFormula, formula::FormulaGrammar::GRAM_NATIVE_UI);

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDoc = XPathHelper::parseExport(
        pXPathFile, m_xSFactory, "xl/externalLinks/_rels/externalLink1.xml.rels");
    CPPUNIT_ASSERT(pDoc);

    // test also the Linux specific bug tdf#121472
    assertXPath(pDoc, "/r:Relationships/r:Relationship", "Target",
                "../tdf138824_externalSource.ods");

    xDocSh->DoClose();
}

void ScExportTest::testTdf129969()
{
    ScDocShellRef xShell = loadDoc("external_hyperlink.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    ScAddress aPos(0, 0, 0);
    const EditTextObject* pEditText = rDoc.GetEditText(aPos);
    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    CPPUNIT_ASSERT(pURLData->GetURL().endsWith("/%23folder/test.ods#Sheet2.B10"));

    xDocSh->DoClose();
}

void ScExportTest::testTdf84874()
{
    ScDocShellRef xShell = loadDoc("tdf84874.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    xShell->DoClose();
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    const ScValidationData* pData = rDoc.GetValidationEntry(1);
    OUString aTitle, aText;
    pData->GetInput(aTitle, aText);
    sal_uInt32 nPromptTitleLen = aTitle.getLength();
    sal_uInt32 nPromptTextLen = aText.getLength();

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(255), nPromptTitleLen);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(255), nPromptTextLen);

    ScValidErrorStyle eErrStyle = SC_VALERR_STOP;
    pData->GetErrMsg(aTitle, aText, eErrStyle);
    sal_uInt32 nErrorTitleLen = aTitle.getLength();
    sal_uInt32 nErrorTextLen = aText.getLength();

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(255), nErrorTitleLen);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(255), nErrorTextLen);

    xDocSh->DoClose();
}

void ScExportTest::testTdf136721_paper_size()
{
    ScDocShellRef xShell = loadDoc("tdf136721_letter_sized_paper.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:pageSetup", "paperSize", "70");
}

void ScExportTest::testTdf139258_rotated_image()
{
    // Check that the topleft position of the image is correct.
    ScDocShellRef xShell = loadDoc(u"tdf139258_rotated_image.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:from/xdr:col", "1");
    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:from/xdr:row", "12");
    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:to/xdr:col", "6");
    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:to/xdr:row", "25");
}

void ScExportTest::testTdf140431()
{
    ScDocShellRef xShell = loadDoc(u"129969-min.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    ScAddress aPos(0, 2, 0);
    const EditTextObject* pEditText = rDoc.GetEditText(aPos);
    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    CPPUNIT_ASSERT(pURLData->GetURL().startsWith("file://ndhlis"));

    xDocSh->DoClose();
}

void ScExportTest::testTdf142264ManyChartsToXLSX()
{
    // The cache size for the test should be small enough, to make sure that some charts get
    // unloaded in the process, and then loaded on demand properly (default is currently 20)
    CPPUNIT_ASSERT_LESS(sal_Int32(40),
        officecfg::Office::Common::Cache::DrawingEngine::OLE_Objects::get());

    ScDocShellRef xDocSh = loadDoc(u"many_charts.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    xDocSh = saveAndReload(xDocSh.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    auto xModel = xDocSh->GetModel();
    css::uno::Reference<css::drawing::XDrawPagesSupplier> xSupplier(xModel,
        css::uno::UNO_QUERY_THROW);
    auto xDrawPages = xSupplier->getDrawPages();

    // No charts (or other objects) on the first sheet, and resp. first draw page
    css::uno::Reference<css::drawing::XDrawPage> xPage(xDrawPages->getByIndex(0),
        css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xPage->getCount());

    // 20 charts on the second sheet, and resp. second draw page
    xPage.set(xDrawPages->getByIndex(1), css::uno::UNO_QUERY_THROW);
    // Without the fix in place, this test would have failed with
    // - Expected: 20
    // - Actual : 0
    // Because only the last 20 charts would get exported, all on the third sheet
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), xPage->getCount());
    for (sal_Int32 i = 0; i < xPage->getCount(); ++i)
    {
        css::uno::Reference<css::beans::XPropertySet> xProps(xPage->getByIndex(i),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::chart2::XChartDocument> xChart(xProps->getPropertyValue("Model"),
            css::uno::UNO_QUERY_THROW);
        const auto xDiagram = xChart->getFirstDiagram();
        CPPUNIT_ASSERT(xDiagram);

        css::uno::Reference<css::chart2::XCoordinateSystemContainer> xCooSysContainer(
            xDiagram, uno::UNO_QUERY_THROW);

        const auto xCooSysSeq = xCooSysContainer->getCoordinateSystems();
        for (const auto& rCooSys : xCooSysSeq)
        {
            css::uno::Reference<css::chart2::XChartTypeContainer> xChartTypeCont(
                rCooSys, uno::UNO_QUERY_THROW);
            uno::Sequence<uno::Reference<chart2::XChartType>> xChartTypeSeq
                = xChartTypeCont->getChartTypes();
            CPPUNIT_ASSERT(xChartTypeSeq.hasElements());
        }
    }

    // 20 charts on the third sheet, and resp. third draw page
    xPage.set(xDrawPages->getByIndex(2), css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), xPage->getCount());
    for (sal_Int32 i = 0; i < xPage->getCount(); ++i)
    {
        css::uno::Reference<css::beans::XPropertySet> xProps(xPage->getByIndex(i),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::chart2::XChartDocument> xChart(xProps->getPropertyValue("Model"),
            css::uno::UNO_QUERY_THROW);
        const auto xDiagram = xChart->getFirstDiagram();
        CPPUNIT_ASSERT(xDiagram);

        css::uno::Reference<css::chart2::XCoordinateSystemContainer> xCooSysContainer(
            xDiagram, uno::UNO_QUERY_THROW);

        const auto xCooSysSeq = xCooSysContainer->getCoordinateSystems();
        for (const auto& rCooSys : xCooSysSeq)
        {
            css::uno::Reference<css::chart2::XChartTypeContainer> xChartTypeCont(
                rCooSys, uno::UNO_QUERY_THROW);
            uno::Sequence<uno::Reference<chart2::XChartType>> xChartTypeSeq
                = xChartTypeCont->getChartTypes();
            CPPUNIT_ASSERT(xChartTypeSeq.hasElements());
        }
    }

    xDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
