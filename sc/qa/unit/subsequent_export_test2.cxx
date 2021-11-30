
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <officecfg/Office/Common.hxx>
#include <config_features.h>

#include <sfx2/sfxmodelfactory.hxx>

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
#include <viewopti.hxx>

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
#include <editeng/fhgtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/justifyitem.hxx>
#include <formula/grammar.hxx>
#include <unotools/useroptions.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <tools/datetime.hxx>
#include <tools/fldunit.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>

#include <test/xmltesttools.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/sheet/GlobalSheetSettings.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/text/XTextColumns.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScExportTest2 : public ScBootstrapFixture, public XmlTestTools
{
protected:
    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;

public:
    ScExportTest2();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testMatrixMultiplicationXLSX();
    void testTdf121260();
    void testTextDirectionXLSX();
    void testTdf120168();
    void testTdf66668();
    void testTdf130108();
    void testTdf76949();
    void testTdf107586();
    void testTdf55417();
    void testTdf129985();
    void testTdf73063();

    xmlDocUniquePtr testTdf95640(std::u16string_view rFileName, sal_Int32 nSourceFormat,
                                 sal_Int32 nDestFormat);
    void testTdf95640_ods_to_xlsx();
    void testTdf95640_ods_to_xlsx_with_standard_list();
    void testTdf95640_xlsx_to_xlsx();
    void testDateAutofilterXLSX();
    void testDateAutofilterODS();
    void testAutofilterColorsODF();
    void testAutofilterColorsOOXML();
    void testAutofilterTop10XLSX();

    void testRefStringXLSX();
    void testRefStringConfigXLSX();
    void testRefStringUnspecified();
    void testHeaderImageODS();
    void testHeaderFooterContentODS();

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
    void testTdf142764();
    void testTdf91634XLSX();
    void testTdf115159();
    void testTdf112567();
    void testTdf122191();
    void testTdf142881();
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
    void testTdf143979();
    void testTdf120502();
    void testTdf131372();
    void testTdf81470();
    void testTdf122331();
    void testTdf83779();
    void testTdf121715_FirstPageHeaderFooterXLSX();
    void testTdf121716_ExportEvenHeaderFooterXLSX();
    void testTdf134459_HeaderFooterColorXLSX();
    void testTdf134817_HeaderFooterTextWith2SectionXLSX();
    void testTdf121718_UseFirstPageNumberXLSX();
    void testHeaderFontStyleXLSX();
    void testTdf135828_Shape_Rect();
    void testTdf123139XLSX();
    void testTdf123353();
    void testTdf140098();
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
    void testTdf142854_GridVisibilityImportXlsxInHeadlessMode();
    void testTdf144642_RowHeightRounding();
    void testTdf145129_DefaultRowHeightRounding();
    void testTdf140431();
    void testCheckboxFormControlXlsxExport();
    void testButtonFormControlXlsxExport();
    void testTdf142929_filterLessThanXLSX();
    void testInvalidNamedRange();
    void testExternalDefinedNameXLSX();
    void testHyperlinkLocationXLSX();
    void testTdf142264ManyChartsToXLSX();
    void testTdf143929MultiColumnToODS();
    void testTdf142578();
    void testTdf130104_XLSXIndent();

    CPPUNIT_TEST_SUITE(ScExportTest2);

    CPPUNIT_TEST(testMatrixMultiplicationXLSX);
    CPPUNIT_TEST(testTdf121260);
    CPPUNIT_TEST(testTextDirectionXLSX);
    CPPUNIT_TEST(testTdf120168);
    CPPUNIT_TEST(testTdf66668);
    CPPUNIT_TEST(testTdf130108);
    CPPUNIT_TEST(testTdf76949);
    CPPUNIT_TEST(testTdf107586);
    CPPUNIT_TEST(testTdf55417);
    CPPUNIT_TEST(testTdf129985);
    CPPUNIT_TEST(testTdf73063);
    CPPUNIT_TEST(testTdf95640_ods_to_xlsx);
    CPPUNIT_TEST(testTdf95640_ods_to_xlsx_with_standard_list);
    CPPUNIT_TEST(testTdf95640_xlsx_to_xlsx);
    CPPUNIT_TEST(testDateAutofilterXLSX);
    CPPUNIT_TEST(testDateAutofilterODS);
    CPPUNIT_TEST(testAutofilterColorsODF);
    CPPUNIT_TEST(testAutofilterColorsOOXML);
    CPPUNIT_TEST(testAutofilterTop10XLSX);

    CPPUNIT_TEST(testRefStringXLSX);
    CPPUNIT_TEST(testRefStringConfigXLSX);
    CPPUNIT_TEST(testRefStringUnspecified);
    CPPUNIT_TEST(testHeaderImageODS);
    CPPUNIT_TEST(testHeaderFooterContentODS);

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
    CPPUNIT_TEST(testTdf142764);
    CPPUNIT_TEST(testTdf91634XLSX);
    CPPUNIT_TEST(testTdf115159);
    CPPUNIT_TEST(testTdf112567);
    CPPUNIT_TEST(testTdf122191);
    CPPUNIT_TEST(testTdf142881);
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
    CPPUNIT_TEST(testTdf143979);
    CPPUNIT_TEST(testTdf120502);
    CPPUNIT_TEST(testTdf131372);
    CPPUNIT_TEST(testTdf81470);
    CPPUNIT_TEST(testTdf122331);
    CPPUNIT_TEST(testTdf83779);
    CPPUNIT_TEST(testTdf121715_FirstPageHeaderFooterXLSX);
    CPPUNIT_TEST(testTdf121716_ExportEvenHeaderFooterXLSX);
    CPPUNIT_TEST(testTdf134459_HeaderFooterColorXLSX);
    CPPUNIT_TEST(testTdf134817_HeaderFooterTextWith2SectionXLSX);
    CPPUNIT_TEST(testTdf121718_UseFirstPageNumberXLSX);
    CPPUNIT_TEST(testHeaderFontStyleXLSX);
    CPPUNIT_TEST(testTdf135828_Shape_Rect);
    CPPUNIT_TEST(testTdf123139XLSX);
    CPPUNIT_TEST(testTdf123353);
    CPPUNIT_TEST(testTdf140098);
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
    CPPUNIT_TEST(testTdf142854_GridVisibilityImportXlsxInHeadlessMode);
    CPPUNIT_TEST(testTdf144642_RowHeightRounding);
    CPPUNIT_TEST(testTdf145129_DefaultRowHeightRounding);
    CPPUNIT_TEST(testTdf140431);
    CPPUNIT_TEST(testCheckboxFormControlXlsxExport);
    CPPUNIT_TEST(testButtonFormControlXlsxExport);
    CPPUNIT_TEST(testTdf142929_filterLessThanXLSX);
    CPPUNIT_TEST(testInvalidNamedRange);
    CPPUNIT_TEST(testExternalDefinedNameXLSX);
    CPPUNIT_TEST(testHyperlinkLocationXLSX);
    CPPUNIT_TEST(testTdf142264ManyChartsToXLSX);
    CPPUNIT_TEST(testTdf143929MultiColumnToODS);
    CPPUNIT_TEST(testTdf142578);
    CPPUNIT_TEST(testTdf130104_XLSXIndent);

    CPPUNIT_TEST_SUITE_END();

private:
    ScDocShellRef loadDocAndSetupModelViewController(std::u16string_view rFileName,
                                                     sal_Int32 nFormat, bool bReadWrite);
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

ScExportTest2::ScExportTest2()
    : ScBootstrapFixture("sc/qa/unit/data")
{
}

void ScExportTest2::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent
        = getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScExportTest2::tearDown()
{
    uno::Reference<lang::XComponent>(m_xCalcComponent, UNO_QUERY_THROW)->dispose();
    test::BootstrapFixture::tearDown();
}

void ScExportTest2::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
    XmlTestTools::registerODFNamespaces(pXmlXPathCtx);
}

void ScExportTest2::testMatrixMultiplicationXLSX()
{
    ScDocShellRef xShell = loadDoc(u"matrix-multiplication.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                     "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    OUString CellFormulaRange = getXPath(pDoc, "/x:worksheet/x:sheetData/x:row[4]/x:c/x:f", "ref");

    // make sure that the CellFormulaRange is G5:G6.
    CPPUNIT_ASSERT_EQUAL(OUString("G5:G6"), CellFormulaRange);

    OUString CellFormulaType = getXPath(pDoc, "/x:worksheet/x:sheetData/x:row[4]/x:c/x:f", "t");

    // make sure that the CellFormulaType is array.
    CPPUNIT_ASSERT_EQUAL(OUString("array"), CellFormulaType);

    xDocSh->DoClose();
}

void ScExportTest2::testRefStringXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"ref_string.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    //make sure ref syntax gets saved for MSO-produced docs
    xDocSh = saveAndReload(&(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScCalcConfig aCalcConfig = rDoc.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL(formula::FormulaGrammar::CONV_XL_A1, aCalcConfig.meStringRefAddressSyntax);

    xDocSh->DoClose();
}

void ScExportTest2::testRefStringConfigXLSX()
{
    // this doc is configured with CalcA1 ref syntax
    ScDocShellRef xDocSh = loadDoc(u"empty.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    xDocSh = saveAndReload(&(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScCalcConfig aConfig = rDoc.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match",
                                 formula::FormulaGrammar::CONV_OOO,
                                 aConfig.meStringRefAddressSyntax);

    xDocSh->DoClose();

    // this doc has no entry for ref syntax
    xDocSh = loadDoc(u"empty-noconf.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open 2nd doc", xDocSh.is());

    ScDocument& rDoc2 = xDocSh->GetDocument();
    aConfig = rDoc2.GetCalcConfig();
    // therefore after import, ref syntax should be set to CalcA1 | ExcelA1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match",
                                 formula::FormulaGrammar::CONV_A1_XL_A1,
                                 aConfig.meStringRefAddressSyntax);

    //set ref syntax to something else than ExcelA1 (native to xlsx format) ...
    aConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_XL_R1C1;
    rDoc2.SetCalcConfig(aConfig);

    ScDocShellRef xNewDocSh = saveAndReload(&(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload 2nd doc", xNewDocSh.is());

    // ... and make sure it got saved
    ScDocument& rDoc3 = xNewDocSh->GetDocument();
    aConfig = rDoc3.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match",
                                 formula::FormulaGrammar::CONV_XL_R1C1,
                                 aConfig.meStringRefAddressSyntax);

    xDocSh->DoClose();
    xNewDocSh->DoClose();
}

void ScExportTest2::testRefStringUnspecified()
{
    ScDocShell* pShell
        = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT | SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS
                         | SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    pShell->DoInitNew();

    ScDocument& rDoc = pShell->GetDocument();
    ScCalcConfig aConfig = rDoc.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default string ref syntax value doesn't match",
                                 formula::FormulaGrammar::CONV_UNSPECIFIED,
                                 aConfig.meStringRefAddressSyntax);

    // change formula syntax (i.e. not string ref syntax) to ExcelA1
    rDoc.SetGrammar(formula::FormulaGrammar::GRAM_NATIVE_XL_A1);

    ScDocShellRef xDocSh = saveAndReload(pShell, FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload doc", xDocSh.is());

    // with string ref syntax at its default value, we should've saved ExcelA1
    ScDocument& rDoc2 = xDocSh->GetDocument();
    aConfig = rDoc2.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match",
                                 formula::FormulaGrammar::CONV_XL_A1,
                                 aConfig.meStringRefAddressSyntax);

    xDocSh->DoClose();
}

void ScExportTest2::testHeaderImageODS()
{
    // Graphic as header background was lost on export.
    ScDocShellRef xShell = loadDoc(u"header-image.", FORMAT_ODS);
    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_ODS);
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xDocSh->GetModel(),
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xPageStyles(xStyleFamilies->getByName("PageStyles"),
                                                       uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName("Default"), uno::UNO_QUERY);

    uno::Reference<graphic::XGraphic> xGraphic;
    xStyle->getPropertyValue("HeaderBackGraphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
    xDocSh->DoClose();
}

void ScExportTest2::testHeaderFooterContentODS()
{
    ScDocShellRef xShell = loadDoc(u"header-footer-content.", FORMAT_ODS);
    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_ODS);
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xDocSh->GetModel(),
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xPageStyles(xStyleFamilies->getByName("PageStyles"),
                                                       uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName("Default"), uno::UNO_QUERY);

    uno::Reference<css::sheet::XHeaderFooterContent> xContent;
    xStyle->getPropertyValue("RightPageHeaderContent") >>= xContent;
    CPPUNIT_ASSERT(xContent.is());
    CPPUNIT_ASSERT_EQUAL(OUString("header"), xContent->getCenterText()->getString());

    xStyle->getPropertyValue("FirstPageHeaderContent") >>= xContent;
    CPPUNIT_ASSERT(xContent.is());
    CPPUNIT_ASSERT_EQUAL(OUString("first page header"), xContent->getCenterText()->getString());

    xStyle->getPropertyValue("RightPageFooterContent") >>= xContent;
    CPPUNIT_ASSERT(xContent.is());
    CPPUNIT_ASSERT_EQUAL(OUString("footer"), xContent->getCenterText()->getString());

    xStyle->getPropertyValue("FirstPageFooterContent") >>= xContent;
    // First page footer content used to be lost upon export.
    CPPUNIT_ASSERT(xContent.is());
    CPPUNIT_ASSERT_EQUAL(OUString("first page footer"), xContent->getCenterText()->getString());

    xDocSh->DoClose();
}

void ScExportTest2::testTextDirectionXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"writingMode.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment", "readingOrder", "1"); //LTR
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[3]/x:alignment", "readingOrder", "2"); //RTL

    xDocSh->DoClose();
}

void ScExportTest2::testTdf121260()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf121260.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    // change formula syntax (i.e. not string ref syntax) to ExcelA1
    rDoc.SetGrammar(formula::FormulaGrammar::GRAM_NATIVE_XL_A1);

    xmlDocUniquePtr pChart1 = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                        "xl/charts/chart1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pChart1);

    // Without the fix in place, this test would have failed with
    // - Expected: Sheet1!$A$1:$A$2
    // - Actual  : sheet1 $A$1:$A$2
    assertXPathContent(pChart1,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:val/c:numRef/c:f",
                       "Sheet1!$A$1:$A$2");
    assertXPathContent(pChart1,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[2]/c:val/c:numRef/c:f",
                       "Sheet1!$B$1:$B$2");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf120168()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf120168.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: left
    // - Actual  : general
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment", "horizontal", "left");
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[3]/x:alignment", "horizontal", "right");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf66668()
{
    // Would hang on exporting without the fix in place
    ScDocShellRef xDocSh = loadDoc(u"tdf66668.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    xDocSh->DoClose();
}

void ScExportTest2::testTdf130108()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf130108.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:b", "val", "1");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:i", "val", "0");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:color", "rgb", "FFFFFFFF");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:sz", "val", "10");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:fill/x:patternFill/x:bgColor", "rgb",
                "FFCC0000");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf76949()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf76949.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pSheet = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                       "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pSheet);

    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c/x:f", "_xlfn.CHISQ.DIST(1,1,1)");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf107586()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf107586.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pSheet = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                       "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pSheet);

    // Without the fix in place, this test would have failed with
    // XPath '/x:worksheet/x:sheetPr/x:tabColor' number of nodes is incorrect
    assertXPath(pSheet, "/x:worksheet/x:sheetPr/x:tabColor", "rgb", "FF9BBB59");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf55417()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf55417.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[1]/x:alignment", 1);
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment", 1);

    xDocSh->DoClose();
}

void ScExportTest2::testTdf129985()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf129985.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode", "m/d/yyyy");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf73063()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf73063.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode",
                "[$-1C1A]dddd\", \"d\". \"mmmm\\ yyyy;@");

    xDocSh->DoClose();
}

xmlDocUniquePtr ScExportTest2::testTdf95640(std::u16string_view rFileName, sal_Int32 nSourceFormat,
                                            sal_Int32 nDestFormat)
{
    ScDocShellRef xShell = loadDoc(rFileName, nSourceFormat);
    CPPUNIT_ASSERT(xShell);

    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), nDestFormat);
    xShell->DoClose();

    return XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
}

void ScExportTest2::testTdf95640_ods_to_xlsx()
{
    // Roundtripping sort options with user defined list to XLSX
    xmlDocUniquePtr pDoc = testTdf95640(u"tdf95640.", FORMAT_ODS, FORMAT_XLSX);

    assertXPath(pDoc, "//x:worksheet/x:autoFilter", "ref", "A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "ref", "A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "customList",
                "Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec");
}

void ScExportTest2::testTdf95640_ods_to_xlsx_with_standard_list()
{
    // Roundtripping sort options with user defined list to XLSX
    xmlDocUniquePtr pDoc = testTdf95640(u"tdf95640_standard_list.", FORMAT_ODS, FORMAT_XLSX);

    assertXPath(pDoc, "//x:worksheet/x:autoFilter", "ref", "A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "ref", "A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "customList",
                "Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday");
}

void ScExportTest2::testTdf95640_xlsx_to_xlsx()
{
    // XLSX Roundtripping sort options with custom sort list - note
    // that compared to ODS source documents above, here we _actually_
    // can use custom lists (beyond the global user defines), like
    // low, medium, high
    xmlDocUniquePtr pDoc = testTdf95640(u"tdf95640.", FORMAT_XLSX, FORMAT_XLSX);

    assertXPath(pDoc, "//x:worksheet/x:autoFilter", "ref", "A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "ref", "A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "customList",
                "Low,Medium,High");
}

void ScExportTest2::testDateAutofilterXLSX()
{
    // XLSX Roundtripping autofilter with date list
    ScDocShellRef xDocSh = loadDoc(u"dateAutofilter.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                     "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:autoFilter", "ref", "A1:B4");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]", "day", "02");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]", "month", "03");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]", "year", "2017");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]",
                "dateTimeGrouping", "day");

    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]", "day", "01");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]", "month", "10");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]", "year", "2014");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]",
                "dateTimeGrouping", "day");

    xDocSh->DoClose();
}

void ScExportTest2::testDateAutofilterODS()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf142231.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "content.xml", FORMAT_ODS);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//table:filter/table:filter-and/table:filter-condition[1]", "value", "Calc");
    assertXPath(pDoc, "//table:filter/table:filter-and/table:filter-condition[2]", "value",
                "2021-05-04");
}

void ScExportTest2::testAutofilterColorsODF()
{
    ScDocShellRef xDocSh = loadDoc(u"autofilter-colors.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "content.xml", FORMAT_ODS);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[1]/table:filter/table:filter-and/"
                "table:filter-condition[1]",
                "value", "#e8f2a1");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[1]/table:filter/table:filter-and/"
                "table:filter-condition[1][@loext:data-type='background-color']");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[1]/table:filter/table:filter-and/"
                "table:filter-condition[2]",
                "value", "#3465a4");
    assertXPath(pDoc, "//table:database-ranges/table:database-range[1]/table:filter/"
                      "table:filter-and/table:filter-condition[2][@loext:data-type='text-color']");

    // tdf#142965 Check "none" value when automatic text color / no fill was selected
    assertXPath(pDoc, "//table:database-ranges/table:database-range[2]/table:filter/"
                      "table:filter-and/"
                      "table:filter-condition[1][@loext:data-type='background-color']");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[2]/table:filter/table:filter-and/"
                "table:filter-condition[1]",
                "value", "transparent");
    assertXPath(pDoc, "//table:database-ranges/table:database-range[3]/table:filter/"
                      "table:filter-and/table:filter-condition[1][@loext:data-type='text-color']");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[3]/table:filter/table:filter-and/"
                "table:filter-condition[1]",
                "value", "window-font-color");
}

void ScExportTest2::testAutofilterColorsOOXML()
{
    {
        ScDocShellRef xDocSh = loadDoc(u"autofilter-colors.", FORMAT_XLSX);
        CPPUNIT_ASSERT(xDocSh.is());
        std::shared_ptr<utl::TempFile> pXPathFile
            = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
        xmlDocUniquePtr pTable1
            = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/tables/table1.xml");
        CPPUNIT_ASSERT(pTable1);
        sal_Int32 nDxfId
            = getXPath(pTable1, "/x:table/x:autoFilter/x:filterColumn/x:colorFilter", "dxfId")
                  .toInt32()
              + 1;

        xmlDocUniquePtr pStyles
            = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/styles.xml");
        CPPUNIT_ASSERT(pStyles);
        OString sDxfXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfId)
                          + "]/x:fill/x:patternFill/x:fgColor");
        assertXPath(pStyles, sDxfXPath, "rgb", "FFFFD7D7");
        xDocSh->DoClose();
    }

    {
        ScDocShellRef xDocSh = loadDoc(u"autofilter-colors-fg.", FORMAT_XLSX);
        CPPUNIT_ASSERT(xDocSh.is());
        std::shared_ptr<utl::TempFile> pXPathFile
            = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
        xmlDocUniquePtr pTable1
            = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/tables/table1.xml");
        CPPUNIT_ASSERT(pTable1);
        sal_Int32 nDxfId
            = getXPath(pTable1, "/x:table/x:autoFilter/x:filterColumn/x:colorFilter", "dxfId")
                  .toInt32()
              + 1;

        xmlDocUniquePtr pStyles
            = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/styles.xml");
        CPPUNIT_ASSERT(pStyles);
        OString sDxfXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfId)
                          + "]/x:fill/x:patternFill/x:fgColor");
        assertXPath(pStyles, sDxfXPath, "rgb", "FF3465A4");
        xDocSh->DoClose();
    }
}

void ScExportTest2::testAutofilterTop10XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf143068_top10filter.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                     "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn", "colId", "0");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:top10", "val", "4");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf88657ODS()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf88657.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "styles.xml", FORMAT_ODS);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//number:fraction", "min-denominator-digits", "3");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf41722()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf41722.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                     "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[1]", "operator", "containsText");
    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[2]", "operator", "containsText");
    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[3]", "operator", "containsText");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf113621()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf113621.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                     "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:conditionalFormatting", "sqref", "A1:A1048576");

    xDocSh->DoClose();
}

void ScExportTest2::testEscapeCharInNumberFormatXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf81939.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xDocSh = saveAndReload(&(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    const sal_Unicode cEuro(8364); // € symbol
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode",
                "00\\ 00\\ 00\\ 00\\ 00");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode",
                "00\\.00\\.00\\.000\\.0"); // tdf#81939
    // "_-* #,##0\ _€_-;\-* #,##0\ _€_-;_-* "- "_€_-;_-@_-" // tdf#81222
    OUString rFormatStrExpected("_-* #,##0\\ _" + OUStringChar(cEuro) + "_-;\\-* #,##0\\ _"
                                + OUStringChar(cEuro) + "_-;_-* \"- \"_" + OUStringChar(cEuro)
                                + "_-;_-@_-");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[4]", "formatCode", rFormatStrExpected);
    // "_-* #,##0" €"_-;\-* #,##0" €"_-;_-* "- €"_-;_-@_-");
    rFormatStrExpected = "_-* #,##0\" " + OUStringChar(cEuro) + "\"_-;\\-* #,##0\" "
                         + OUStringChar(cEuro) + "\"_-;_-* \"- " + OUStringChar(cEuro)
                         + "\"_-;_-@_-";
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[5]", "formatCode", rFormatStrExpected);
    // remove escape char in fraction
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[6]", "formatCode",
                "# ?/?;[RED]\\-# #/#####");

    xDocSh->DoClose();
}

void ScExportTest2::testNatNumInNumberFormatXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf79398_NatNum5.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    xDocSh = saveAndReload(&(*xDocSh), FORMAT_XLSX); // Convert [NatNum5] to [DBNum2] in Chinese
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode",
                "[DBNum2][$-804]General;[RED][DBNum2][$-804]General");

    xDocSh->DoClose();
}

void ScExportTest2::testExponentWithoutSignFormatXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf102370_ExponentWithoutSign.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    xDocSh = saveAndReload(&(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xDocSh = saveAndReload(&(*xDocSh), FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    sal_uInt32 nNumberFormat = rDoc.GetNumberFormat(0, 0, 0);
    const SvNumberformat* pNumberFormat = rDoc.GetFormatTable()->GetEntry(nNumberFormat);
    const OUString& rFormatStr = pNumberFormat->GetFormatstring();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format lost exponent without sign during Excel export",
                                 OUString("0.00E0"), rFormatStr);

    xDocSh->DoClose();
}

void ScExportTest2::testExtendedLCIDXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf36038_ExtendedLCID.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    xDocSh = saveAndReload(&(*xDocSh), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/styles.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    // Check export
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode",
                "[$-107041E]dd\\-mm\\-yyyy");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode",
                "[$-D07041E]dd\\-mm\\-yyyy");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[4]", "formatCode",
                "[$-1030411]dd\\-mm\\-ee");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[5]", "formatCode",
                "[$-1B030411]dd\\-mm\\-ee");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[6]", "formatCode",
                "[$-108040D]dd\\-mm\\-yyyy");
    //assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[7]", "formatCode", "[$-108040D]dd\\-mm\\-yyyy");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[7]", "formatCode",
                "[$-1060401]dd\\-mm\\-yyyy");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[8]", "formatCode",
                "[$-2060401]dd\\-mm\\-yyyy");

    // Check import
    ScDocument& rDoc = xDocSh->GetDocument();
    SvNumberFormatter* pNumFormatter = rDoc.GetFormatTable();
    const OUString aLang[5] = { "[$-41E]", "[$-411]", "[$-40D]", "[$-401]", "[$-500]" };
    const OUString aCalendar[5] = { "[~buddhist]DD-MM-YYYY", "DD-MM-EE", "[~jewish]DD-MM-YYYY",
                                    "[~hijri]DD-MM-YYYY", "[~dangi]YYYY/MM/DD" };
    // Note: ja-JP Gengou calendar is an implicit secondary (non-gregorian)
    // calendar, the explicit [~gengou] calendar modifier does not need to be
    // present, the E and EE keywords are used instead of YY and YYYY.
    for (sal_Int16 nCol = 1; nCol <= 2; nCol++)
    {
        for (sal_Int16 nRow = 1; nRow <= 4; nRow++)
        {
            sal_uInt32 nNumberFormat = rDoc.GetNumberFormat(nCol, nRow, 0);
            const SvNumberformat* pNumberFormat = pNumFormatter->GetEntry(nNumberFormat);
            const OUString& rFormatStr = pNumberFormat->GetFormatstring();
            const OUString aExpectedFormatStr
                = aLang[nRow - 1] + ((nCol == 2 && nRow != 3) ? OUString("[NatNum1]") : OUString())
                  + aCalendar[nRow - 1];

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format lost extended LCID during Excel export",
                                         aExpectedFormatStr, rFormatStr);
        }
    }

    xDocSh->DoClose();
}

void ScExportTest2::testHiddenRepeatedRowsODS()
{
    ScDocShellRef xDocSh = loadDoc(u"empty.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    {
        ScDocument& rDoc = xDocSh->GetDocument();
        rDoc.SetRowHidden(0, 20, 0, true);
    }

    xDocSh = saveAndReload(&(*xDocSh), FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    SCROW nFirstRow = 0;
    SCROW nLastRow = 0;
    bool bHidden = rDoc.RowHidden(0, 0, &nFirstRow, &nLastRow);
    CPPUNIT_ASSERT(bHidden);
    CPPUNIT_ASSERT_EQUAL(SCROW(0), nFirstRow);
    CPPUNIT_ASSERT_EQUAL(SCROW(20), nLastRow);
    xDocSh->DoClose();
}

void ScExportTest2::testHyperlinkTargetFrameODS()
{
    ScDocShellRef xDocSh = loadDoc(u"hyperlink_frame.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const EditTextObject* pEditText = rDoc.GetEditText(ScAddress(2, 5, 0));
    CPPUNIT_ASSERT(pEditText);

    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the URL data.", pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to get the URL data.", text::textfield::Type::URL,
                                 pData->GetClassId());

    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    OUString aTargetFrame = pURLData->GetTargetFrame();
    CPPUNIT_ASSERT_EQUAL(OUString("_blank"), aTargetFrame);

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "content.xml", FORMAT_ODS);
    CPPUNIT_ASSERT(pDoc);
    OUString aTargetFrameExport
        = getXPath(pDoc,
                   "/office:document-content/office:body/office:spreadsheet/table:table/"
                   "table:table-row[2]/table:table-cell[2]/text:p/text:a",
                   "target-frame-name");
    CPPUNIT_ASSERT_EQUAL(OUString("_blank"), aTargetFrameExport);

    xDocSh->DoClose();
}

void ScExportTest2::testOpenDocumentAsReadOnly()
{
    ScDocShellRef xDocSh = loadDoc(u"open-as-read-only.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh->IsSecurityOptOpenReadOnly());
    ScDocShellRef xDocSh2 = saveAndReload(xDocSh.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh2->IsSecurityOptOpenReadOnly());
    xDocSh->DoClose();
    xDocSh2->DoClose();
}

void ScExportTest2::testKeepSettingsOfBlankRows()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf41425.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // saved blank row with not default setting in A2
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row", 2);

    xDocSh->DoClose();
}

void ScExportTest2::testTdf133595()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf133595.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // without the fix in place, mc:AlternateContent would have been added to sheet1
    assertXPath(pSheet, "/x:worksheet/mc:AlternateContent", 0);

    xDocSh->DoClose();
}

void ScExportTest2::testTdf134769()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf134769.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // without the fix in place, the legacyDrawing would have been exported after the checkbox
    // and Excel would have claimed the document is corrupted
    // Use their ids to check the order
    assertXPath(pSheet, "/x:worksheet/x:drawing", "id", "rId2");
    assertXPath(pSheet, "/x:worksheet/x:legacyDrawing", "id", "rId3");
    assertXPath(pSheet,
                "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/"
                "mc:Choice/x:control",
                "id", "rId4");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf106181()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf106181.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet,
                "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/"
                "mc:Choice/x:control",
                "name", "Check Box");
    assertXPath(pSheet,
                "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/"
                "mc:Choice/x:control/x:controlPr",
                "altText", "Check Box 1");

    xmlDocUniquePtr pDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(
        pDrawing,
        "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr",
        "name", "Check Box 1");
    assertXPath(
        pDrawing,
        "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr",
        "descr", "Check Box");
    assertXPath(
        pDrawing,
        "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr",
        "hidden", "0");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf105272()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf105272.", FORMAT_XLSX);
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

void ScExportTest2::testTdf118990()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf118990.", FORMAT_XLSX);
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

void ScExportTest2::testTdf121612()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf121612.", FORMAT_ODS);
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

void ScExportTest2::testTdf112936()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf112936.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(
        *this, *xDocSh, m_xSFactory, "xl/pivotCache/pivotCacheDefinition1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:pivotCacheDefinition", "recordCount", "4");
    assertXPath(pDoc, "//x:pivotCacheDefinition", "createdVersion", "3");

    xDocSh->DoClose();
}

void ScExportTest2::testXltxExport()
{
    // Create new document
    ScDocShell* pShell
        = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT | SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS
                         | SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    pShell->DoInitNew();

    // Export as template and check content type
    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *pShell, m_xSFactory,
                                                     "[Content_Types].xml", FORMAT_XLTX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/ContentType:Types/ContentType:Override[@PartName='/xl/workbook.xml']",
                "ContentType",
                "application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml");
}

void ScExportTest2::testPivotCacheAfterExportXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"numgroup_example.", FORMAT_ODS);
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

void ScExportTest2::testTdf114969XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"sheet_name_with_dots.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                     "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[1]", "location", "'1.1.1.1'!C1");
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[2]", "location", "'1.1.1.1'!C2");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf115192XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"test_115192.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(
        *this, *xDocSh, m_xSFactory, "xl/drawings/_rels/drawing1.xml.rels", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']", "TargetMode",
                "External");
    assertXPathNoAttribute(pDoc, "/rels:Relationships/rels:Relationship[@Id='rId2']", "TargetMode");
    assertXPath(pDoc, "/rels:Relationships/rels:Relationship[@Id='rId3']", "TargetMode",
                "External");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf142764()
{
    ScDocShellRef xShell = loadDoc(u"tdf142764.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell);

    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pSheet
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:headerFooter", "differentOddEven", "true");
    assertXPath(pSheet, "/x:worksheet/x:headerFooter", "differentFirst", "true");

    xShell->DoClose();
}

void ScExportTest2::testTdf91634XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"image_hyperlink.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:pic/xdr:nvPicPr/xdr:cNvPr/a:hlinkClick", 1);

    xmlDocUniquePtr pXmlRels
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                "https://www.google.com/");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "TargetMode",
                "External");

    xDocSh->DoClose();
}

void ScExportTest2::testValidationCopyPaste()
{
    ScDocShellRef xDocSh = loadDoc(u"validation-copypaste.", FORMAT_ODS);
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

void ScExportTest2::testTdf115159()
{
    ScDocShellRef xShell = loadDoc(u"tdf115159.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());
    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xShell->DoClose();

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/workbook.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName", 1);

    xDocSh->DoClose();
}

void ScExportTest2::testTdf112567()
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

    ScDocShellRef xShell = loadDoc(u"tdf112567.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());
    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xShell->DoClose();

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/workbook.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName", 1);

    xDocSh->DoClose();
}

void ScExportTest2::testTdf122191()
{
    // Set the system locale to Hungarian
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString("hu-HU");
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    ScDocShellRef xShell = loadDoc(u"tdf122191.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocument& rDoc = xShell->GetDocument();
    CPPUNIT_ASSERT_EQUAL(OUString("IGAZ"), rDoc.GetString(0, 0, 0));

    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xShell->DoClose();

    ScDocument& rDoc2 = xDocSh->GetDocument();
    // Without the fix in place, this test would have failed with
    // - Expected: IGAZ
    // - Actual  : BOOL00AN
    CPPUNIT_ASSERT_EQUAL(OUString("IGAZ"), rDoc2.GetString(0, 0, 0));

    xDocSh->DoClose();
}

void ScExportTest2::testTdf142881()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf142881.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf142881.xlsx", xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDrawing1
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing1);

    // Verify that the shapes are rotated and positioned in the expected way
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:from/xdr:col", "11");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:from/xdr:row", "0");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:to/xdr:col", "12");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:to/xdr:row", "19");

    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:from/xdr:col", "2");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:from/xdr:row", "8");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:to/xdr:col", "7");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:to/xdr:row", "10");

    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:from/xdr:col", "10");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:from/xdr:row", "9");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:to/xdr:col", "11");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:to/xdr:row", "26");

    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:from/xdr:col", "2");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:from/xdr:row", "17");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:to/xdr:col", "8");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:to/xdr:row", "19");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf112567b()
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

    ScDocShellRef xShell = loadDoc(u"tdf112567.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());
    ScDocShellRef xDocSh = saveAndReload(xShell.get(), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    xShell->DoClose();

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory, "xl/workbook.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName", 1);

    //and it contains "," instead of ";"
    assertXPathContent(pDoc, "/x:workbook/x:definedNames/x:definedName[1]",
                       "Sheet1!$A:$A,Sheet1!$1:$1");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf123645XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"chart_hyperlink.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:graphicFrame/xdr:nvGraphicFramePr/xdr:cNvPr/"
                "a:hlinkClick",
                1);
    assertXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:graphicFrame/xdr:nvGraphicFramePr/xdr:cNvPr/"
                "a:hlinkClick",
                1);
    assertXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:graphicFrame/xdr:nvGraphicFramePr/xdr:cNvPr/"
                "a:hlinkClick",
                1);

    xmlDocUniquePtr pXmlRels
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "TargetMode",
                "External");
    assertXPathNoAttribute(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId3']",
                           "TargetMode");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId5']", "TargetMode",
                "External");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                "file:///C:/TEMP/test.xlsx");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId3']", "Target",
                "#Sheet2!A1");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId5']", "Target",
                "https://bugs.documentfoundation.org/show_bug.cgi?id=123645");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf125173XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"text_box_hyperlink.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr/a:hlinkClick", 1);

    xmlDocUniquePtr pXmlRels
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                "http://www.google.com/");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "TargetMode",
                "External");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf79972XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf79972.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink", "ref", "A1");

    xmlDocUniquePtr pXmlRels
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/_rels/sheet1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "Target",
                "https://bugs.documentfoundation.org/show_bug.cgi?id=79972");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "TargetMode", "External");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf126024XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"hyperlink_formula.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink", "ref", "A2");

    xmlDocUniquePtr pXmlRels
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/_rels/sheet1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "Target",
                "https://bugs.documentfoundation.org/");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "TargetMode", "External");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf126177XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"hyperlink_export.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink", "location", "Munka1!A5");

    xmlDocUniquePtr pXmlRels
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/_rels/sheet1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    OUString aTarget = getXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "Target");
    CPPUNIT_ASSERT(aTarget.endsWith("test.xlsx"));
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "TargetMode", "External");

    xDocSh->DoClose();
}

void ScExportTest2::testCommentTextVAlignment()
{
    // Testing comment text alignments.
    ScDocShellRef xShell = loadDoc(u"CommentTextVAlign.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pVmlDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/vmlDrawing1.vml");
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/xml/v:shape/xx:ClientData/xx:TextVAlign", "Center");

    xShell->DoClose();
}

void ScExportTest2::testCommentTextHAlignment()
{
    // Testing comment text alignments.
    ScDocShellRef xShell = loadDoc(u"CommentTextHAlign.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pVmlDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/vmlDrawing1.vml");
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/xml/v:shape/xx:ClientData/xx:TextHAlign", "Center");

    xShell->DoClose();
}

void ScExportTest2::testRotatedImageODS()
{
    // Error was, that the length values in shapes were not
    // written in the given unit into the file.
    css::uno::Reference<css::sheet::XGlobalSheetSettings> xGlobalSheetSettings
        = css::sheet::GlobalSheetSettings::create(comphelper::getProcessComponentContext());
    xGlobalSheetSettings->setMetric(static_cast<sal_Int16>(FieldUnit::MM));

    ScDocShellRef xDocSh = loadDoc(u"tdf103092_RotatedImage.", FORMAT_ODS, true);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pTemp = saveAs(xDocSh.get(), FORMAT_ODS);
    CPPUNIT_ASSERT(pTemp);
    xmlDocUniquePtr pXmlDoc = XPathHelper::parseExport(pTemp, m_xSFactory, "content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    const OUString sTransform = getXPath(pXmlDoc,
                                         "/office:document-content/office:body/office:spreadsheet/"
                                         "table:table/table:shapes/draw:frame",
                                         "transform");
    // Attribute transform has the structure skew (...) rotate (...) translate (x y)
    // parts are separated by blank
    OUString sTranslate(sTransform.copy(sTransform.lastIndexOf('(')));
    sTranslate = sTranslate.copy(1, sTranslate.getLength() - 2); // remove '(' and ')'
    const OUString sX(sTranslate.getToken(0, ' '));
    const OUString sY(sTranslate.getToken(1, ' '));
    CPPUNIT_ASSERT(sX.endsWith("mm"));
    CPPUNIT_ASSERT(sY.endsWith("mm"));

    xDocSh->DoClose();
}

void ScExportTest2::testTdf128976()
{
    ScDocShellRef xShell = loadDoc(u"tdf128976.", FORMAT_XLS);
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

void ScExportTest2::testTdf143979()
{
    // Create an empty worksheet
    css::uno::Reference<css::frame::XDesktop2> xDesktop
        = css::frame::Desktop::create(comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop);

    css::uno::Sequence args{ comphelper::makePropertyValue("Hidden", true) };

    css::uno::Reference<css::lang::XComponent> xComponent
        = xDesktop->loadComponentFromURL("private:factory/scalc", "_blank", 0, args);
    CPPUNIT_ASSERT(xComponent);

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShellRef xShell = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xShell);

    ScDocument& rDoc = xShell->GetDocument();

    OUString aCode = "YYYY-MM\"\"MMM-DDNN";
    sal_Int32 nCheckPos;
    SvNumFormatType nType;
    sal_uInt32 nFormat;
    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    pFormatter->PutEntry(aCode, nCheckPos, nType, nFormat);

    ScPatternAttr aNewAttrs(rDoc.GetPool());
    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));

    rDoc.ApplyPattern(0, 0, 0, aNewAttrs);

    rDoc.SetString(ScAddress(0, 0, 0), "08/30/2021");

    CPPUNIT_ASSERT_EQUAL(OUString("2021-08Aug-30Mon"), rDoc.GetString(ScAddress(0, 0, 0)));

    ScDocShellRef pDocSh = saveAndReload(&(*xShell), FORMAT_ODS);
    CPPUNIT_ASSERT(pDocSh.is());

    ScDocument& rDoc2 = pDocSh->GetDocument();

    // Without the fix in place, this test would have failed with
    // - Expected: 2021-08Aug-30Mon
    // - Actual  : 2021-A-30Mon
    CPPUNIT_ASSERT_EQUAL(OUString("2021-08Aug-30Mon"), rDoc2.GetString(ScAddress(0, 0, 0)));
}

void ScExportTest2::testTdf120502()
{
    // Create an empty worksheet; resize last column on its first sheet; export to XLSX, and check
    // that the last exported column number is correct
    css::uno::Reference<css::frame::XDesktop2> xDesktop
        = css::frame::Desktop::create(comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop);

    css::uno::Sequence args{ comphelper::makePropertyValue("Hidden", true) };

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

void ScExportTest2::testTdf131372()
{
    ScDocShellRef xShell = loadDoc(u"tdf131372.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell);

    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pSheet
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c[1]/x:f", "NA()");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c[2]/x:f", "#N/A");

    xShell->DoClose();
}
void ScExportTest2::testTdf81470()
{
    ScDocShellRef xShell = loadDoc(u"tdf81470.", FORMAT_XLS);
    CPPUNIT_ASSERT(xShell);

    //without the fix in place, it would have crashed at export time
    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    //also check revisions are exported
    xmlDocUniquePtr pHeaders
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/revisions/revisionHeaders.xml");
    CPPUNIT_ASSERT(pHeaders);

    assertXPath(pHeaders, "/x:headers/x:header[1]", "dateTime", "2014-07-11T13:46:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[1]", "userName", "Kohei Yoshida");
    assertXPath(pHeaders, "/x:headers/x:header[2]", "dateTime", "2014-07-11T18:38:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[2]", "userName", "Kohei Yoshida");
    assertXPath(pHeaders, "/x:headers/x:header[3]", "dateTime", "2014-07-11T18:43:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[3]", "userName", "Kohei Yoshida");

    xShell->DoClose();
}

void ScExportTest2::testTdf122331()
{
    ScDocShellRef xShell = loadDoc(u"tdf122331.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell);

    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pSheet
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:sheetPr", "filterMode", "true");
    assertXPath(pSheet, "/x:worksheet/x:autoFilter", "ref", "A1:B761");
    assertXPath(pSheet, "/x:worksheet/x:autoFilter/x:filterColumn", "colId", "1");

    xShell->DoClose();
}

void ScExportTest2::testTdf83779()
{
    // Roundtripping TRUE/FALSE constants (not functions) must convert them to functions
    ScDocShellRef xShell = loadDoc(u"tdf83779.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell);

    auto pXPathFile = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xmlDocUniquePtr pVmlDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/x:worksheet/x:sheetData/x:row[1]/x:c/x:f", "FALSE()");
    assertXPathContent(pVmlDrawing, "/x:worksheet/x:sheetData/x:row[2]/x:c/x:f", "TRUE()");

    xShell->DoClose();
}

void ScExportTest2::testTdf121715_FirstPageHeaderFooterXLSX()
{
    // Check if first page header and footer are exported properly
    ScDocShellRef xShell = loadDoc(u"tdf121715.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:headerFooter", "differentFirst", "true");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:firstHeader", "&CFirst Page Header");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:firstFooter", "&CFirst Page Footer");

    xShell->DoClose();
}

void ScExportTest2::testTdf121716_ExportEvenHeaderFooterXLSX()
{
    // Header and footer on even pages should be exported properly
    // If there are separate odd/even header, but only 1 footer for all pages (this is possible only in LibreOffice)
    //  then the footer will be duplicated to have the same footer separately for even/odd pages

    ScDocShellRef xShell = loadDoc(u"tdf121716_EvenHeaderFooter.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:headerFooter", "differentOddEven", "true");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader",
                       "&Lodd/right&Cpage&Rheader");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter", "&Lboth&C&12page&Rfooter");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenHeader",
                       "&Lpage&Cheader&Reven/left");
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

void ScExportTest2::testTdf134459_HeaderFooterColorXLSX()
{
    // Colors in header and footer should be exported, and imported properly
    ScDocShellRef xShell = loadDoc(u"tdf134459_HeaderFooterColor.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                     "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader",
                       "&L&Kc06040l&C&K4c3789c&Rr");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter",
                       "&Ll&C&K64cf5fc&R&Kcd15aar");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf134817_HeaderFooterTextWith2SectionXLSX()
{
    // Header/footer text with multiple selection should be exported, and imported properly
    ScDocShellRef xShell = loadDoc(u"tdf134817_HeaderFooterTextWith2Section.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                     "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader",
                       "&L&\"Abadi,Regular\"&11aaa&\"Bembo,Regular\"&20bbb");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter",
                       "&R&\"Cambria,Regular\"&14camb&\"Dante,Regular\"&18dant");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf121718_UseFirstPageNumberXLSX()
{
    // If "First page number" is not checked then useFirstPageNumb, and firstPageNumber should not be exported.
    ScDocShellRef xShell = loadDoc(u"tdf121718_UseFirstPageNumber.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:pageSetup", "useFirstPageNumber", "true");
    assertXPath(pDoc, "/x:worksheet/x:pageSetup", "firstPageNumber", "10");

    pDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet2.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPathNoAttribute(pDoc, "/x:worksheet/x:pageSetup", "useFirstPageNumber");
    assertXPathNoAttribute(pDoc, "/x:worksheet/x:pageSetup", "firstPageNumber");

    xDocSh->DoClose();
}

void ScExportTest2::testHeaderFontStyleXLSX()
{
    ScDocShellRef xShell = loadDoc(u"tdf134826.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocument& rDoc = xShell->GetDocument();
    SfxStyleSheetBase* pStyleSheet
        = rDoc.GetStyleSheetPool()->Find(rDoc.GetPageStyle(0), SfxStyleFamily::Page);
    const SfxItemSet& rItemSet = pStyleSheet->GetItemSet();
    const ScPageHFItem& rHFItem = rItemSet.Get(ATTR_PAGE_HEADERRIGHT);
    const EditTextObject* pTextObj = rHFItem.GetLeftArea();

    std::vector<EECharAttrib> rLst;

    // first line is bold.
    pTextObj->GetCharAttribs(0, rLst);
    bool bHasBold = std::any_of(rLst.begin(), rLst.end(), [](const EECharAttrib& rAttrib) {
        return rAttrib.pAttr->Which() == EE_CHAR_WEIGHT
               && static_cast<const SvxWeightItem&>(*rAttrib.pAttr).GetWeight() == WEIGHT_BOLD;
    });
    CPPUNIT_ASSERT_MESSAGE("First line should be bold.", bHasBold);

    // second line is italic.
    pTextObj->GetCharAttribs(1, rLst);
    bool bHasItalic = std::any_of(rLst.begin(), rLst.end(), [](const EECharAttrib& rAttrib) {
        return rAttrib.pAttr->Which() == EE_CHAR_ITALIC
               && static_cast<const SvxPostureItem&>(*rAttrib.pAttr).GetPosture() == ITALIC_NORMAL;
    });
    CPPUNIT_ASSERT_MESSAGE("Second line should be italic.", bHasItalic);

    xShell->DoClose();
}

void ScExportTest2::testTdf135828_Shape_Rect()
{
    if (!IsDefaultDPI())
        return;
    // tdf#135828 Check that the width and the height of rectangle of the shape is correct.
    // tdf#123613 Check the positioning, and allow massive rounding errors because of the back and
    // forth conversion between emu and hmm.
    ScDocShellRef xShell = loadDoc(u"tdf135828_Shape_Rect.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    double nXPosOfTopleft
        = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:off", "x")
              .toDouble();
    double nYPosOfTopleft
        = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:off", "y")
              .toDouble();
    double nWidth
        = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:ext", "cx")
              .toDouble();
    double nHeight
        = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:ext", "cy")
              .toDouble();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(854640, nXPosOfTopleft, 10000);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-570600, nYPosOfTopleft, 10000);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(294840, nWidth, 10000);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1988280, nHeight, 10000);
}

void ScExportTest2::testTdf123139XLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf123139_applyAlignment.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const ScPatternAttr* pAttr = rDoc.GetPattern(0, 0, 0); //A1

    {
        const SvxHorJustifyItem& rJustify = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Repeat, rJustify.GetValue());
    }

    pAttr = rDoc.GetPattern(0, 1, 0); //A2

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        const SvxHorJustifyItem& rJustify = static_cast<const SvxHorJustifyItem&>(rItem);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Center, rJustify.GetValue());
    }

    {
        const ScProtectionAttr& rItem = pAttr->GetItem(ATTR_PROTECTION);
        CPPUNIT_ASSERT(rItem.GetProtection());
        CPPUNIT_ASSERT(!rItem.GetHideFormula());
    }

    pAttr = rDoc.GetPattern(2, 0, 0); //C1

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        const SvxHorJustifyItem& rJustify = static_cast<const SvxHorJustifyItem&>(rItem);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Standard, rJustify.GetValue());
    }

    {
        const ScProtectionAttr& rItem = pAttr->GetItem(ATTR_PROTECTION);
        CPPUNIT_ASSERT(rItem.GetProtection());
        CPPUNIT_ASSERT(rItem.GetHideFormula());
    }

    pAttr = rDoc.GetPattern(2, 1, 0); //C2

    {
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        const SvxHorJustifyItem& rJustify = static_cast<const SvxHorJustifyItem&>(rItem);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Block, rJustify.GetValue());
    }

    {
        const ScProtectionAttr& rItem = pAttr->GetItem(ATTR_PROTECTION);
        CPPUNIT_ASSERT(!rItem.GetProtection());
        CPPUNIT_ASSERT(!rItem.GetHideFormula());
    }

    xDocSh->DoClose();
}

void ScExportTest2::testTdf123353()
{
    ScDocShellRef xShell = loadDoc(u"tdf123353.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:autoFilter/x:filterColumn/x:filters", "blank", "1");

    xShell->DoClose();
}

void ScExportTest2::testTdf140098()
{
    ScDocShellRef xShell = loadDoc(u"tdf140098.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:autoFilter/x:filterColumn/x:filters", "blank", "1");

    xShell->DoClose();
}

void ScExportTest2::testTdf133688_precedents()
{
    // tdf#133688 Check that we do not export detective shapes.
    ScDocShellRef xShell = loadDoc(u"tdf133688_dont_save_precedents_to_xlsx.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    // We do not export any shapes.
    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor[1]", 0);
}

void ScExportTest2::testTdf91251_missingOverflowRoundtrip()
{
    // tdf#91251 check whether textBox overflow property (horzOverflow and vertOverflow) is
    // getting preserved after roundtrip
    ScDocShellRef xShell = loadDoc(u"tdf91251_missingOverflowRoundtrip.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);

    xmlDocUniquePtr pDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr", "horzOverflow",
                "clip");
    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr", "horzOverflow",
                "clip");
}

void ScExportTest2::testTdf137000_handle_upright()
{
    // tdf#106197 When exporting the "upright" attribute, we must set
    // TextPreRotateAngle to 0.
    // (Upright is an xml attribute of xdr:txBody/a:bodyPr. It is set when
    // in a textbox menu we choose: do not rotate this element.)
    ScDocShellRef xShell = loadDoc(u"tdf137000_export_upright.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pDrawing
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr", "rot",
                "-5400000");
}

void ScExportTest2::testTdf126305_DataValidatyErrorAlert()
{
    ScDocShellRef xShell = loadDoc(u"tdf126305.", FORMAT_ODS);
    CPPUNIT_ASSERT(xShell.is());

    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[1]", "errorStyle", "stop");
    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[2]", "errorStyle",
                "warning");
    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[3]", "errorStyle",
                "information");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf76047_externalLink()
{
    ScDocShellRef pShell = loadDoc(u"tdf76047_externalLink.", FORMAT_XLSX);
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

void ScExportTest2::testTdf87973_externalLinkSkipUnuseds()
{
    ScDocShellRef pShell = loadDoc(u"tdf87973_externalLinkSkipUnuseds.", FORMAT_ODS);
    CPPUNIT_ASSERT(pShell.is());

    // try to load data from external link: tdf132105_external.ods
    // that file has to be in the same directory as tdf87973_externalLinkSkipUnuseds.ods
    pShell->ReloadAllLinks();
    ScDocument& rDoc = pShell->GetDocument();

    // change external link to: 87973_externalSource.ods
    OUString aFormula = rDoc.GetFormula(3, 1, 0);
    auto nIdxOfFilename = aFormula.indexOf("tdf132105_external.ods");
    aFormula = aFormula.replaceAt(nIdxOfFilename, 22, u"87973_externalSource.ods");
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
    aFormula = rDoc.GetFormula(3, 2, 0);
    nIdxOfFilename = aFormula.indexOf("tdf132105_external.ods");
    aFormula = aFormula.replaceAt(nIdxOfFilename, 22, u"87973_externalSource.ods");
    nIdxOfFile = aFormula.indexOf("file");

    aFormula = aFormula.replaceAt(nIdxOfFile, nIdxOfFilename - nIdxOfFile, aTempFilename);
    rDoc.SetFormula(ScAddress(3, 2, 0), aFormula, formula::FormulaGrammar::GRAM_NATIVE_UI);

    // save and load back
    ScDocShellRef pDocSh = saveAndReload(&(*pShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(pDocSh.is());

    // check if the new filename is present in the link (and not replaced by '[2]')
    ScDocument& rDoc2 = pDocSh->GetDocument();
    OUString aFormula2 = rDoc2.GetFormula(3, 1, 0);
    CPPUNIT_ASSERT(aFormula2.indexOf("tdf132105_external.ods") < 0);
    CPPUNIT_ASSERT(aFormula2.indexOf("87973_externalSource.ods") >= 0);
    aFormula2 = rDoc2.GetFormula(3, 2, 0);
    CPPUNIT_ASSERT(aFormula2.indexOf("tdf132105_external.ods") < 0);
    CPPUNIT_ASSERT(aFormula2.indexOf("87973_externalSource.ods") >= 0);

    pDocSh->DoClose();
}

void ScExportTest2::testTdf51022_lostPrintRange()
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

void ScExportTest2::testTdf138741_externalLinkSkipUnusedsCrash()
{
    ScDocShellRef xShell = loadDoc(u"tdf138741_externalLinkSkipUnusedsCrash.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell);

    //without the fix in place, it would have crashed at export time
    ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    xShell->DoClose();
}

void ScExportTest2::testTdf138824_linkToParentDirectory()
{
    ScDocShellRef xShell = loadDoc(u"childDir/tdf138824_linkToParentDirectory.", FORMAT_ODS);
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
    OUString aFormula = rDoc.GetFormula(3, 1, 0);
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
    assertXPath(pDoc, "/rels:Relationships/rels:Relationship", "Target",
                "../tdf138824_externalSource.ods");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf129969()
{
    ScDocShellRef xShell = loadDoc(u"external_hyperlink.", FORMAT_ODS);
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

void ScExportTest2::testTdf84874()
{
    ScDocShellRef xShell = loadDoc(u"tdf84874.", FORMAT_ODS);
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

void ScExportTest2::testTdf136721_paper_size()
{
    ScDocShellRef xShell = loadDoc(u"tdf136721_letter_sized_paper.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);
    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:pageSetup", "paperSize", "70");
}

void ScExportTest2::testTdf139258_rotated_image()
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

ScDocShellRef ScExportTest2::loadDocAndSetupModelViewController(std::u16string_view rFileName,
                                                                sal_Int32 nFormat, bool bReadWrite)
{
    uno::Reference<frame::XDesktop2> xDesktop
        = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    // create a frame
    Reference<frame::XFrame> xTargetFrame = xDesktop->findFrame("_blank", 0);
    CPPUNIT_ASSERT(xTargetFrame.is());

    // 1. Open the document
    ScDocShellRef xDocSh = loadDoc(rFileName, nFormat, bReadWrite);
    CPPUNIT_ASSERT_MESSAGE(
        OString("Failed to load " + OUStringToOString(rFileName, RTL_TEXTENCODING_UTF8)).getStr(),
        xDocSh.is());

    uno::Reference<frame::XModel2> xModel2 = xDocSh->GetModel();
    CPPUNIT_ASSERT(xModel2.is());

    Reference<frame::XController2> xController = xModel2->createDefaultViewController(xTargetFrame);
    CPPUNIT_ASSERT(xController.is());

    // introduce model/view/controller to each other
    xController->attachModel(xModel2);
    xModel2->connectController(xController);
    xTargetFrame->setComponent(xController->getComponentWindow(), xController);
    xController->attachFrame(xTargetFrame);
    xModel2->setCurrentController(xController);

    return xDocSh;
}

void ScExportTest2::testTdf142854_GridVisibilityImportXlsxInHeadlessMode()
{
    // Tests are running in Headless mode
    // Import an ods file with 'Hide' global grid visibility setting.
    ScDocShellRef xShell
        = loadDocAndSetupModelViewController(u"tdf126541_GridOffGlobally.", FORMAT_ODS, true);
    CPPUNIT_ASSERT(!xShell->GetDocument().GetViewOptions().GetOption(VOPT_GRID));

    // To avoid regression, in headless mode leave the bug tdf126541
    // It means Sheet based grid line visibility setting will overwrite the global setting.
    // If there is only 1 sheet in the document, it will not result visible problems.
    xShell = loadDocAndSetupModelViewController(u"tdf126541_GridOff.", FORMAT_XLSX, true);
    CPPUNIT_ASSERT(!xShell->GetDocument().GetViewOptions().GetOption(VOPT_GRID));
}

void ScExportTest2::testTdf144642_RowHeightRounding()
{
    // MS Excel round down row heights to 0.75pt
    // MS Excel can save a row height of 28.35pt, but will display it as a row height of 27.75pt.
    // Calc simulates this roundings but only if the xlsx file was saved in MS Excel.

    ScDocShellRef xShell = loadDoc(u"tdf144642_RowHeight_10mm_SavedByCalc.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());
    ScDocument& rDoc = xShell->GetDocument();
    // 10mm == 567 twips == 28.35pt
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(567), rDoc.GetRowHeight(0, 0));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(567 * 26), rDoc.GetRowHeight(0, 25, 0, true));
    xShell->DoClose();

    xShell = loadDoc(u"tdf144642_RowHeight_28.35pt_SavedByExcel.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());
    ScDocument& rDoc2 = xShell->GetDocument();
    // 555twips == 27.75pt == 9.79mm
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(555), rDoc2.GetRowHeight(0, 0));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(555 * 26), rDoc2.GetRowHeight(0, 25, 0, true));
    xShell->DoClose();
}

void ScExportTest2::testTdf145129_DefaultRowHeightRounding()
{
    // MS Excel round down row heights to 0.75pt
    // Same as Tdf144642 but with default row height.

    ScDocShellRef xShell
        = loadDoc(u"tdf145129_DefaultRowHeight_28.35pt_SavedByExcel.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());
    ScDocument& rDoc = xShell->GetDocument();
    // 555twips == 27.75pt == 9.79mm
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(555), rDoc.GetRowHeight(0, 0));
    CPPUNIT_ASSERT_EQUAL(sal_uLong(555 * 52), rDoc.GetRowHeight(0, 51, 0, true));
    xShell->DoClose();
}

void ScExportTest2::testTdf140431()
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

void ScExportTest2::testCheckboxFormControlXlsxExport()
{
    if (!IsDefaultDPI())
        return;
    // Given a document that has a checkbox form control:
    ScDocShellRef xShell = loadDoc(u"checkbox-form-control.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    // When exporting to XLSX:
    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    // Then make sure its VML markup is written and it has a correct position + size:
    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/drawings/vmlDrawing1.vml");
    // Without the fix in place, this test would have failed as there was no such stream.
    CPPUNIT_ASSERT(pDoc);
    assertXPathContent(pDoc, "/xml/v:shape/xx:ClientData/xx:Anchor", "1, 22, 3, 3, 3, 30, 6, 1");
}

void ScExportTest2::testButtonFormControlXlsxExport()
{
    // Given a document that has a checkbox form control:
    ScDocShellRef xShell = loadDoc(u"button-form-control.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());

    // When exporting to XLSX:
    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xShell), FORMAT_XLSX);

    // Then make sure its control markup is written and it has a correct position + size:
    xmlDocUniquePtr pDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    // Without the fix in place, this test would have failed with:
    // - XPath '//x:anchor/x:from/xdr:col' not found
    // i.e. the control markup was missing, the button was lost on export.
    assertXPathContent(pDoc, "//x:anchor/x:from/xdr:col", "1");
    assertXPathContent(pDoc, "//x:anchor/x:from/xdr:row", "3");
    assertXPathContent(pDoc, "//x:anchor/x:to/xdr:col", "3");
    assertXPathContent(pDoc, "//x:anchor/x:to/xdr:row", "7");

    // Also make sure that an empty macro attribute is not written.
    // Without the fix in place, this test would have failed with:
    // - XPath '//x:controlPr' unexpected 'macro' attribute
    // i.e. macro in an xlsx file was not omitted, which is considered invalid by Excel.
    assertXPathNoAttribute(pDoc, "//x:controlPr", "macro");
}

void ScExportTest2::testTdf142929_filterLessThanXLSX()
{
    // Document contains a standard filter with '<' condition.
    ScDocShellRef xDocSh = loadDoc(u"tdf142929.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                     "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "//x:customFilters/x:customFilter", "val", "2");
    assertXPath(pDoc, "//x:customFilters/x:customFilter", "operator", "lessThan");

    xDocSh->DoClose();
}

void ScExportTest2::testInvalidNamedRange()
{
    // Given a document which has a named range (myname) that refers to the "1" external link, but
    // the link's type is xlPathMissing, when importing that document:
    ScDocShellRef xDocSh = loadDoc(u"invalid-named-range.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    // Then make sure that named range is ignored, as "1" can't be resolved, and exporting it back
    // to XLSX (without the xlPathMissing link) would corrupt the document:
    uno::Reference<beans::XPropertySet> xDocProps(xDocSh->GetModel(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNamedRanges(xDocProps->getPropertyValue("NamedRanges"),
                                                        uno::UNO_QUERY);
    // Without the fix in place, this test would have failed, we didn't ignore the problematic named
    // range on import.
    CPPUNIT_ASSERT(!xNamedRanges->hasByName("myname"));
}

void ScExportTest2::testExternalDefinedNameXLSX()
{
    ScDocShellRef xShell = loadDoc(u"tdf144397.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xShell.is());
    ScDocShellRef xDocSh = saveAndReload(&(*xShell), FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    xDocSh->ReloadAllLinks();
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // "January"
    {
        const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(1, 1, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::String, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(OUString("January"), aRes.maString.getString());
    }
    // "March"
    {
        const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(1, 3, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::String, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(OUString("March"), aRes.maString.getString());
    }
    // "Empty = #N/A"
    {
        const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(1, 5, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::Error, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(OUString(""), aRes.maString.getString());
    }
    // "June"
    {
        const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(1, 6, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::String, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(OUString("June"), aRes.maString.getString());
    }

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(
        *this, *xDocSh, m_xSFactory, "xl/externalLinks/externalLink1.xml", FORMAT_XLSX);

    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:externalLink/x:externalBook/x:sheetNames/x:sheetName", "val", "Munka1");
    assertXPath(pDoc, "/x:externalLink/x:externalBook/x:definedNames/x:definedName", "name",
                "MonthNames");
    // TODO: no need for the [1] external document identifier
    assertXPath(pDoc, "/x:externalLink/x:externalBook/x:definedNames/x:definedName", "refersTo",
                "[1]Munka1!$A$2:$A$13");
    assertXPath(pDoc, "/x:externalLink/x:externalBook/x:sheetDataSet/x:sheetData", "sheetId", "0");
    assertXPath(pDoc, "/x:externalLink/x:externalBook/x:sheetDataSet/x:sheetData/x:row[2]", "r",
                "3");
    assertXPathContent(
        pDoc, "/x:externalLink/x:externalBook/x:sheetDataSet/x:sheetData/x:row[2]/x:cell/x:v",
        "February");

    xDocSh->DoClose();
}

void ScExportTest2::testHyperlinkLocationXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf143220.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    xmlDocUniquePtr pDoc = XPathHelper::parseExport2(*this, *xDocSh, m_xSFactory,
                                                     "xl/worksheets/sheet1.xml", FORMAT_XLSX);
    CPPUNIT_ASSERT(pDoc);

    // tdf#143220 link to sheet not valid without cell reference
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[@ref='A1']", "location", "Sheet2!A1");

    // tdf#145079 link with defined name target didn't work because Calc added "A1" at the end
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[@ref='A2']", "location", "name");
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[@ref='A3']", "location", "db");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf142264ManyChartsToXLSX()
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

void ScExportTest2::testTdf143929MultiColumnToODS()
{
    ScDocShellRef xDocSh = loadDoc(u"two-col-shape.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh);

    {
        css::uno::Reference<css::drawing::XDrawPagesSupplier> xSupplier(xDocSh->GetModel(),
                                                                        css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::drawing::XDrawPage> xPage(xSupplier->getDrawPages()->getByIndex(0),
                                                           css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::container::XIndexAccess> xIndexAccess(xPage,
                                                                       css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::drawing::XShape> xShape(xIndexAccess->getByIndex(0),
                                                         css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::beans::XPropertySet> xProps(xShape, css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::text::XTextColumns> xCols(xProps->getPropertyValue("TextColumns"),
                                                           css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCols->getColumnCount());
        css::uno::Reference<css::beans::XPropertySet> xColProps(xCols, css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(css::uno::Any(sal_Int32(1000)),
                             xColProps->getPropertyValue("AutomaticDistance"));
    }

    auto tempFile = exportTo(xDocSh.get(), FORMAT_ODS);
    xDocSh = load(tempFile->GetURL(), FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh);

    {
        css::uno::Reference<css::drawing::XDrawPagesSupplier> xSupplier(xDocSh->GetModel(),
                                                                        css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::drawing::XDrawPage> xPage(xSupplier->getDrawPages()->getByIndex(0),
                                                           css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::container::XIndexAccess> xIndexAccess(xPage,
                                                                       css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::drawing::XShape> xShape(xIndexAccess->getByIndex(0),
                                                         css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::beans::XPropertySet> xProps(xShape, css::uno::UNO_QUERY_THROW);

        // Without the fix in place, this would have failed with:
        //   An uncaught exception of type com.sun.star.uno.RuntimeException
        //   - unsatisfied query for interface of type com.sun.star.text.XTextColumns!
        css::uno::Reference<css::text::XTextColumns> xCols(xProps->getPropertyValue("TextColumns"),
                                                           css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCols->getColumnCount());
        css::uno::Reference<css::beans::XPropertySet> xColProps(xCols, css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(css::uno::Any(sal_Int32(1000)),
                             xColProps->getPropertyValue("AutomaticDistance"));
    }

    xDocSh->DoClose();

    xmlDocUniquePtr pXmlDoc = XPathHelper::parseExport(tempFile, m_xSFactory, "content.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the fix in place, this would have failed with:
    //   - Expected: 1
    //   - Actual  : 0
    //   - In <>, XPath '/office:document-content/office:automatic-styles/style:style[@style:family='graphic']/
    //     style:graphic-properties/style:columns' number of nodes is incorrect
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[@style:family='graphic']/"
        "style:graphic-properties/style:columns",
        "column-count", "2");
    // Only test that "column-gap" attribute exists, not its value that depends on locale (cm, in)
    getXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[@style:family='graphic']/"
        "style:graphic-properties/style:columns",
        "column-gap");
}

void ScExportTest2::testTdf142578()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf142578.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh);

    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // Get DxfId for color filter
    sal_Int32 nDxfIdColorFilter
        = getXPath(pSheet, "/x:worksheet/x:autoFilter/x:filterColumn/x:colorFilter", "dxfId")
              .toInt32()
          + 1;

    // Get DxfId for conditional formatting
    sal_Int32 nDxfIdCondFormat
        = getXPath(pSheet, "/x:worksheet/x:conditionalFormatting/x:cfRule", "dxfId").toInt32() + 1;

    // Ensure they are using different dxfs
    CPPUNIT_ASSERT_MESSAGE("dxfID's should be different!", nDxfIdColorFilter != nDxfIdCondFormat);

    // Check colors used by these dxfs
    xmlDocUniquePtr pStyles = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/styles.xml");
    CPPUNIT_ASSERT(pStyles);

    OString sDxfColorFilterXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfIdColorFilter)
                                 + "]/x:fill/x:patternFill/x:fgColor");
    assertXPath(pStyles, sDxfColorFilterXPath, "rgb", "FF81D41A");

    OString sDxfCondFormatXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfIdCondFormat)
                                + "]/x:fill/x:patternFill/x:bgColor");
    assertXPath(pStyles, sDxfCondFormatXPath, "rgb", "FFFFCCCC");

    xDocSh->DoClose();
}

void ScExportTest2::testTdf130104_XLSXIndent()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf130104_indent.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh);

    // Resave the xlsx file without any modification.
    std::shared_ptr<utl::TempFile> pXPathFile
        = ScBootstrapFixture::exportTo(&(*xDocSh), FORMAT_XLSX);
    xmlDocUniquePtr pSheet
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);
    xmlDocUniquePtr pStyle = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/styles.xml");
    CPPUNIT_ASSERT(pStyle);

    // Check to see whether the indents remain the same as the original ones:

    // Get the style index number for cell A1
    sal_Int32 nCellA1StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]/x:c[1]", "s").toInt32() + 1;
    // The indent for cell A1 should be 0
    OString sStyleA1XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA1StyleIndex) + "]/x:alignment";
    // (if this assertion fails, you should first check whether there is no style index set for this cell)
    assertXPath(pStyle, sStyleA1XPath, "indent", "0");

    sal_Int32 nCellA3StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]/x:c[1]", "s").toInt32() + 1;
    // The indent for cell A3 should be 1
    OString sStyleA3XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA3StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA3XPath, "indent", "1");

    sal_Int32 nCellA6StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA6XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA6StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA6XPath, "indent", "2");

    sal_Int32 nCellA9StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[9]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA9XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA9StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA9XPath, "indent", "3");

    sal_Int32 nCellA12StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[12]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA12XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA12StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA12XPath, "indent", "4");

    sal_Int32 nCellA15StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[15]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA15XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA15StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA15XPath, "indent", "5");

    sal_Int32 nCellA18StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[18]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA18XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA18StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA18XPath, "indent", "6");

    sal_Int32 nCellA21StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA21XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA21StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA21XPath, "indent", "7");

    sal_Int32 nCellA24StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[24]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA24XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA24StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA24XPath, "indent", "8");

    sal_Int32 nCellA27StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[27]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA27XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA27StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA27XPath, "indent", "9");

    sal_Int32 nCellA30StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[30]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA30XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA30StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA30XPath, "indent", "10");

    xDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScExportTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
