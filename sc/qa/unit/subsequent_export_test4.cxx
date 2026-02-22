/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <stlpool.hxx>
#include <validat.hxx>
#include <scresid.hxx>
#include <dbdata.hxx>
#include <subtotalparam.hxx>
#include <globstr.hrc>
#include <dpobject.hxx>

#include <comphelper/processfactory.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>
#include <editeng/justifyitem.hxx>
#include <formula/grammar.hxx>
#include <tools/fldunit.hxx>
#include <svl/numformat.hxx>

#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/sheet/GlobalSheetSettings.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScExportTest4 : public ScModelTestBase
{
public:
    ScExportTest4();

    void testTdf151484(TestFilter eFilter);
};

ScExportTest4::ScExportTest4()
    : ScModelTestBase(u"sc/qa/unit/data"_ustr)
{
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf126177XLSX)
{
    createScDoc("xlsx/hyperlink_export.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink", "location", u"Munka1!A5");

    xmlDocUniquePtr pXmlRels = parseExport(u"xl/worksheets/_rels/sheet1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlRels);
    OUString aTarget = getXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "Target");
    CPPUNIT_ASSERT(aTarget.endsWith("test.xlsx"));
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "TargetMode", u"External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testCommentTextVAlignment)
{
    // Testing comment text alignments.
    createScDoc("ods/CommentTextVAlign.ods");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pVmlDrawing = parseExport(u"xl/drawings/vmlDrawing1.vml"_ustr);
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/xml/v:shape/xx:ClientData/xx:TextVAlign", u"Center");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testCommentTextHAlignment)
{
    // Testing comment text alignments.
    createScDoc("ods/CommentTextHAlign.ods");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pVmlDrawing = parseExport(u"xl/drawings/vmlDrawing1.vml"_ustr);
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/xml/v:shape/xx:ClientData/xx:TextHAlign", u"Center");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testRotatedImageODS)
{
    // Error was, that the length values in shapes were not
    // written in the given unit into the file.
    css::uno::Reference<css::sheet::XGlobalSheetSettings> xGlobalSheetSettings
        = css::sheet::GlobalSheetSettings::create(comphelper::getProcessComponentContext());
    xGlobalSheetSettings->setMetric(static_cast<sal_Int16>(FieldUnit::MM));

    createScDoc("ods/tdf103092_RotatedImage.ods");

    save(TestFilter::ODS);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
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
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf120177)
{
    createScDoc("xls/tdf120177.xls");

    // Error: unexpected attribute "form:input-required"
    skipValidation();

    save(TestFilter::ODS);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // no attribute 'value' exist
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table/office:forms/"
                "form:form/form:radio[1]",
                "value", u"1");
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table/office:forms/"
                "form:form/form:radio[2]",
                "value", u"2");
    const OUString sGroupName1 = getXPath(pXmlDoc,
                                          "/office:document-content/office:body/office:spreadsheet/"
                                          "table:table/office:forms/form:form/form:radio[1]",
                                          "group-name");
    const OUString sGroupName2 = getXPath(pXmlDoc,
                                          "/office:document-content/office:body/office:spreadsheet/"
                                          "table:table/office:forms/form:form/form:radio[2]",
                                          "group-name");
    CPPUNIT_ASSERT_EQUAL(sGroupName1, sGroupName2);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf85553)
{
    createScDoc("ods/tdf85553.ods");

    saveAndReload(TestFilter::XLS);

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 4.5
    // - Actual  : #N/A
    CPPUNIT_ASSERT_EQUAL(u"4.5"_ustr, pDoc->GetString(ScAddress(2, 2, 0)));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf151484_ODS) { testTdf151484(TestFilter::ODS); }

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf151484_XLS)
{
    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 1
    // - Failed on filter: MS Excel 97
    testTdf151484(TestFilter::XLS);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf151484_XLSX) { testTdf151484(TestFilter::XLSX); }

void ScExportTest4::testTdf151484(TestFilter eFilter)
{
    createScDoc("ods/tdf151484.ods");

    const OString sFailedMessage
        = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilter).toUtf8();

    saveAndReload(eFilter);

    ScDocument* pDoc = getScDoc();

    const ScValidationData* pData = pDoc->GetValidationEntry(1);
    CPPUNIT_ASSERT(pData);

    std::vector<ScTypedStrData> aList;
    pData->FillSelectionList(aList, ScAddress(0, 1, 0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), size_t(4), aList.size());
    for (size_t j = 0; j < 4; ++j)
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), double(j + 1), aList[j].GetValue());
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf143979)
{
    createScDoc();
    {
        ScDocument* pDoc = getScDoc();
        OUString aCode = u"YYYY-MM\"\"MMM-DDNN"_ustr;
        sal_Int32 nCheckPos;
        SvNumFormatType nType;
        sal_uInt32 nFormat;
        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        pFormatter->PutEntry(aCode, nCheckPos, nType, nFormat);
        ScPatternAttr aNewAttrs(pDoc->getCellAttributeHelper());
        aNewAttrs.ItemSetPut(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));
        pDoc->ApplyPattern(0, 0, 0, aNewAttrs);
        pDoc->SetString(ScAddress(0, 0, 0), u"08/30/2021"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"2021-08Aug-30Mon"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    }

    saveAndReload(TestFilter::ODS);

    ScDocument* pDoc = getScDoc();
    // Without the fix in place, this test would have failed with
    // - Expected: 2021-08Aug-30Mon
    // - Actual  : 2021-A-30Mon
    CPPUNIT_ASSERT_EQUAL(u"2021-08Aug-30Mon"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf128976)
{
    createScDoc("xls/tdf128976.xls");

    saveAndReload(TestFilter::XLS);

    ScDocument* pDoc = getScDoc();

    // Trying to save very small fractional default column width to XLS (where only integer values
    // between 0 and 255 are allowed as default) resulted in negative (-1) value after correction,
    // and was written as 65535 (invalid default width). As the result, all columns had large width
    // when reopened: 28415 (and Excel warned about invalid format).
    const sal_uInt16 nColumn0Width = pDoc->GetColWidth(SCCOL(0), SCTAB(0), false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(45), nColumn0Width);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf120502)
{
    // Create an empty worksheet; resize last column on its first sheet; export to XLSX, and check
    // that the last exported column number is correct
    createScDoc();

    ScDocument* pDoc = getScDoc();
    const SCCOL nMaxCol = pDoc->MaxCol(); // 0-based

    const auto nOldWidth = pDoc->GetColWidth(nMaxCol, 0);
    pDoc->SetColWidth(nMaxCol, 0, nOldWidth + 100);

    save(TestFilter::XLSX);
    xmlDocUniquePtr pSheet1 = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet1);

    // This was 1025 when nMaxCol+1 was 1024
    assertXPath(pSheet1, "/x:worksheet/x:cols/x:col", "max", OUString::number(nMaxCol + 1));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf158921_exportPivotTable)
{
    createScDoc("ods/tdf158921.ods");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 1
    // - validation error in OOXML export: Errors: 1
    saveAndReload(TestFilter::XLSX);

    ScDocument* pDoc = getScDoc();

    // There should be a pivot table
    CPPUNIT_ASSERT(pDoc->HasPivotTable());

    // DP collection is not lost after export and has one entry
    ScDPCollection* pDPColl = pDoc->GetDPCollection();
    CPPUNIT_ASSERT(pDPColl);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDPColl->GetCount());
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf131372)
{
    createScDoc("ods/tdf131372.ods");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c[1]/x:f", u"NA()");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c[2]/x:f", u"#N/A");
}
CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf81470)
{
    createScDoc("xls/tdf81470.xls");

    //without the fix in place, it would have crashed at export time
    save(TestFilter::XLSX);

    //also check revisions are exported
    xmlDocUniquePtr pHeaders = parseExport(u"xl/revisions/revisionHeaders.xml"_ustr);
    CPPUNIT_ASSERT(pHeaders);

    assertXPath(pHeaders, "/x:headers/x:header[1]", "dateTime", u"2014-07-11T13:46:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[1]", "userName", u"Kohei Yoshida");
    assertXPath(pHeaders, "/x:headers/x:header[2]", "dateTime", u"2014-07-11T18:38:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[2]", "userName", u"Kohei Yoshida");
    assertXPath(pHeaders, "/x:headers/x:header[3]", "dateTime", u"2014-07-11T18:43:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[3]", "userName", u"Kohei Yoshida");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf162262)
{
    createScDoc("xlsx/subtotal-above.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:sheetPr/x:outlinePr", "summaryBelow", u"0");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf162262_summarybelow)
{
    createScDoc("ods/tdf162262_summarybelow.ods");

    ScDocument* pDoc = getScDoc();
    ScDBCollection* pDBCollection = pDoc->GetDBCollection();
    CPPUNIT_ASSERT(pDBCollection);
    {
        const ScDBData* pDBData = pDBCollection->GetDBAtArea(0, 0, 0, 1, 13);
        CPPUNIT_ASSERT(pDBData);
        ScSubTotalParam aParam;
        pDBData->GetSubTotalParam(aParam);
        CPPUNIT_ASSERT(!aParam.bSummaryBelow);
    }

    saveAndReload(TestFilter::ODS);
    pDoc = getScDoc();
    pDBCollection = pDoc->GetDBCollection();
    CPPUNIT_ASSERT(pDBCollection);
    {
        const ScDBData* pDBData = pDBCollection->GetDBAtArea(0, 0, 0, 1, 13);
        CPPUNIT_ASSERT(pDBData);
        ScSubTotalParam aParam;
        pDBData->GetSubTotalParam(aParam);
        CPPUNIT_ASSERT(!aParam.bSummaryBelow);
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf122331)
{
    createScDoc("ods/tdf122331.ods");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:sheetPr", "filterMode", u"true");
    assertXPath(pSheet, "/x:worksheet/x:autoFilter", "ref", u"A1:B761");
    assertXPath(pSheet, "/x:worksheet/x:autoFilter/x:filterColumn", "colId", u"1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf83779)
{
    // Roundtripping TRUE/FALSE constants (not functions) must convert them to functions
    createScDoc("xlsx/tdf83779.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pVmlDrawing = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/x:worksheet/x:sheetData/x:row[1]/x:c/x:f", u"FALSE()");
    assertXPathContent(pVmlDrawing, "/x:worksheet/x:sheetData/x:row[2]/x:c/x:f", u"TRUE()");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf121715_FirstPageHeaderFooterXLSX)
{
    // Check if first page header and footer are exported properly
    createScDoc("xlsx/tdf121715.xlsx");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:headerFooter", "differentFirst", u"true");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:firstHeader", u"&CFirst Page Header");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:firstFooter", u"&CFirst Page Footer");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf121716_ExportEvenHeaderFooterXLSX)
{
    // Header and footer on even pages should be exported properly
    // If there are separate odd/even header, but only 1 footer for all pages (this is possible only in LibreOffice)
    //  then the footer will be duplicated to have the same footer separately for even/odd pages

    createScDoc("ods/tdf121716_EvenHeaderFooter.ods");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:headerFooter", "differentOddEven", u"true");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader",
                       u"&Lodd/right&Cpage&Rheader");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter", u"&Lboth&C&12page&Rfooter");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenHeader",
                       u"&Lpage&Cheader&Reven/left");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenFooter",
                       u"&Lboth&C&12page&Rfooter");

    pDoc = parseExport(u"xl/worksheets/sheet2.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:headerFooter", "differentOddEven", u"true");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader", u"&Coddh");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter", u"&Coddf");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenHeader", u"&Cevenh");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenFooter", u"&Levenf");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf134459_HeaderFooterColorXLSX)
{
    // Colors in header and footer should be exported, and imported properly
    createScDoc("xlsx/tdf134459_HeaderFooterColor.xlsx");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader",
                       u"&L&Kc06040l&C&K4c3789c&Rr");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter",
                       u"&Ll&C&K64cf5fc&R&Kcd15aar");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf134817_HeaderFooterTextWith2SectionXLSX)
{
    // Header/footer text with multiple selection should be exported, and imported properly
    createScDoc("xlsx/tdf134817_HeaderFooterTextWith2Section.xlsx");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader",
                       u"&L&\"Abadi,Regular\"&11aaa&\"Bembo,Regular\"&20bbb");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter",
                       u"&R&\"Cambria,Regular\"&14camb&\"Dante,Regular\"&18dant");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf121718_UseFirstPageNumberXLSX)
{
    // If "First page number" is not checked then useFirstPageNumb, and firstPageNumber should not be exported.
    createScDoc("ods/tdf121718_UseFirstPageNumber.ods");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:pageSetup", "useFirstPageNumber", u"true");
    assertXPath(pDoc, "/x:worksheet/x:pageSetup", "firstPageNumber", u"10");

    pDoc = parseExport(u"xl/worksheets/sheet2.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPathNoAttribute(pDoc, "/x:worksheet/x:pageSetup", "useFirstPageNumber");
    assertXPathNoAttribute(pDoc, "/x:worksheet/x:pageSetup", "firstPageNumber");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testHeaderFontStyleXLSX)
{
    createScDoc("xlsx/tdf134826.xlsx");

    ScDocument* pDoc = getScDoc();
    SfxStyleSheetBase* pStyleSheet
        = pDoc->GetStyleSheetPool()->Find(pDoc->GetPageStyle(0), SfxStyleFamily::Page);
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
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf154445_unused_pagestyles)
{
    createScDoc("ods/tdf108188_pagestyle.ods");

    // Check if the user defined page style is present
    static constexpr OUString aTestPageStyle = u"TestPageStyle"_ustr;
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(aTestPageStyle, pDoc->GetPageStyle(0));

    // Change page style to default so the user defined one is not used anymore
    pDoc->SetPageStyle(0, ScResId(STR_STYLENAME_STANDARD));

    // Save and reload the document to check if the unused page styles are still present
    saveAndReload(TestFilter::ODS);
    pDoc = getScDoc();

    // Without the accompanying fix in place, the unused page styles don't exist anymore
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    CPPUNIT_ASSERT(pStylePool->Find(aTestPageStyle, SfxStyleFamily::Page));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf135828_Shape_Rect)
{
    if (!IsDefaultDPI())
        return;
    // tdf#135828 Check that the width and the height of rectangle of the shape is correct.
    // tdf#123613 Check the positioning, and allow massive rounding errors because of the back and
    // forth conversion between emu and hmm.
    createScDoc("xlsx/tdf135828_Shape_Rect.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pDrawing = parseExport(u"xl/drawings/drawing1.xml"_ustr);
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

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf123139XLSX)
{
    createScDoc("xlsx/tdf123139_applyAlignment.xlsx");

    ScDocument* pDoc = getScDoc();
    const ScPatternAttr* pAttr = pDoc->GetPattern(0, 0, 0); //A1

    {
        const SvxHorJustifyItem& rJustify = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Repeat, rJustify.GetValue());
    }

    pAttr = pDoc->GetPattern(0, 1, 0); //A2

    {
        const SvxHorJustifyItem& rJustify = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Center, rJustify.GetValue());
    }

    {
        const ScProtectionAttr& rItem = pAttr->GetItem(ATTR_PROTECTION);
        CPPUNIT_ASSERT(rItem.GetProtection());
        CPPUNIT_ASSERT(!rItem.GetHideFormula());
    }

    pAttr = pDoc->GetPattern(2, 0, 0); //C1

    {
        const SvxHorJustifyItem& rJustify = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Standard, rJustify.GetValue());
    }

    {
        const ScProtectionAttr& rItem = pAttr->GetItem(ATTR_PROTECTION);
        CPPUNIT_ASSERT(rItem.GetProtection());
        CPPUNIT_ASSERT(rItem.GetHideFormula());
    }

    pAttr = pDoc->GetPattern(2, 1, 0); //C2

    {
        const SvxHorJustifyItem& rJustify = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Block, rJustify.GetValue());
    }

    {
        const ScProtectionAttr& rItem = pAttr->GetItem(ATTR_PROTECTION);
        CPPUNIT_ASSERT(!rItem.GetProtection());
        CPPUNIT_ASSERT(!rItem.GetHideFormula());
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf123353)
{
    createScDoc("xlsx/tdf123353.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:autoFilter/x:filterColumn/x:filters", "blank", u"1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf140098)
{
    createScDoc("ods/tdf140098.ods");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:autoFilter/x:filterColumn/x:filters", "blank", u"1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf133688_precedents)
{
    // tdf#133688 Check that we do not export detective shapes.
    createScDoc("ods/tdf133688_dont_save_precedents_to_xlsx.ods");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDrawing = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDrawing);

    // We do not export any shapes.
    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor[1]", 0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf91251_missingOverflowRoundtrip)
{
    // tdf#91251 check whether textBox overflow property (horzOverflow and vertOverflow) is
    // getting preserved after roundtrip
    createScDoc("xlsx/tdf91251_missingOverflowRoundtrip.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pDrawing = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr", "horzOverflow",
                u"clip");
    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr", "horzOverflow",
                u"clip");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf137000_handle_upright)
{
    // Upright is an xml attribute of xdr:txBody/a:bodyPr. It is set when in a textbox menu we
    // choose, 'do not rotate this element'. Implementations are in tdf#106197 with followup
    // tdf#137000. tdf#149538, tdf#149551 improve the implementation to export 'upright' instead
    // of workaround 'rot'.
    createScDoc("xlsx/tdf137000_export_upright.xlsx");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDrawing = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr", "upright",
                u"1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf126305_DataValidatyErrorAlert)
{
    createScDoc("ods/tdf126305.ods");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[1]", "errorStyle", u"stop");
    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[2]", "errorStyle",
                u"warning");
    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[3]", "errorStyle",
                u"information");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf76047_externalLink)
{
    createScDoc("xlsx/tdf76047_externalLink.xlsx");

    // load data from external links. (tdf76047_externalLinkSource.ods)
    // that file has to be in the same directory as tdf76047_externalLink.xlsx
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->ReloadAllLinks();
    ScDocument* pDoc = getScDoc();

    // compare the data loaded from external links with the expected result stored in the test file
    for (int nCol = 1; nCol <= 5; nCol++)
    {
        for (int nRow = 3; nRow <= 5; nRow++)
        {
            OUString aStr1 = pDoc->GetString(ScAddress(nCol, nRow, 0));
            OUString aStr2 = pDoc->GetString(ScAddress(nCol, nRow + 5, 0));
            OUString aStr3 = pDoc->GetString(ScAddress(nCol, nRow + 11, 0));

            CPPUNIT_ASSERT_EQUAL(aStr1, aStr3);
            CPPUNIT_ASSERT_EQUAL(aStr2, aStr3);
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf87973_externalLinkSkipUnuseds)
{
    createScDoc("ods/tdf87973_externalLinkSkipUnuseds.ods");

    // try to load data from external link: tdf132105_external.ods
    // that file has to be in the same directory as tdf87973_externalLinkSkipUnuseds.ods
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->ReloadAllLinks();
    ScDocument* pDoc = getScDoc();

    // change external link to: 87973_externalSource.ods
    OUString aFormula = pDoc->GetFormula(3, 1, 0);
    auto nIdxOfFilename = aFormula.indexOf("tdf132105_external.ods");
    aFormula = aFormula.replaceAt(nIdxOfFilename, 22, u"87973_externalSource.ods");
    auto nIdxOfFile = aFormula.indexOf("file");

    // saveAndReload save the file to a temporary directory
    // the link must be changed to point to that directory
    OUString aTempFilename = utl::CreateTempURL();
    auto nIdxOfTmpFile = aTempFilename.lastIndexOf('/');
    aTempFilename = aTempFilename.copy(0, nIdxOfTmpFile + 1);

    aFormula = aFormula.replaceAt(nIdxOfFile, nIdxOfFilename - nIdxOfFile, aTempFilename);
    pDoc->SetFormula(ScAddress(3, 1, 0), aFormula, formula::FormulaGrammar::GRAM_NATIVE_UI);

    // tdf#138832: test the same thing with singleref link
    aFormula = pDoc->GetFormula(3, 2, 0);
    nIdxOfFilename = aFormula.indexOf("tdf132105_external.ods");
    aFormula = aFormula.replaceAt(nIdxOfFilename, 22, u"87973_externalSource.ods");
    nIdxOfFile = aFormula.indexOf("file");

    aFormula = aFormula.replaceAt(nIdxOfFile, nIdxOfFilename - nIdxOfFile, aTempFilename);
    pDoc->SetFormula(ScAddress(3, 2, 0), aFormula, formula::FormulaGrammar::GRAM_NATIVE_UI);

    // save and load back
    saveAndReload(TestFilter::XLSX);

    // check if the new filename is present in the link (and not replaced by '[2]')
    pDoc = getScDoc();
    OUString aFormula2 = pDoc->GetFormula(3, 1, 0);
    CPPUNIT_ASSERT(aFormula2.indexOf("tdf132105_external.ods") < 0);
    CPPUNIT_ASSERT(aFormula2.indexOf("87973_externalSource.ods") >= 0);
    aFormula2 = pDoc->GetFormula(3, 2, 0);
    CPPUNIT_ASSERT(aFormula2.indexOf("tdf132105_external.ods") < 0);
    CPPUNIT_ASSERT(aFormula2.indexOf("87973_externalSource.ods") >= 0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf51022_lostPrintRange)
{
    createScDoc("ods/tdf87973_externalLinkSkipUnuseds.ods");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->ReloadAllLinks();
    ScDocument* pDoc = getScDoc();

    //Add print ranges
    ScRange aRange1(1, 2, 0, 3, 4, 0);
    ScRange aRange2(1, 6, 0, 3, 7, 0);
    pDoc->AddPrintRange(0, aRange1);
    pDoc->AddPrintRange(0, aRange2);

    // save and load back
    saveAndReload(TestFilter::ODS);

    // check if the same print ranges are present
    pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2), pDoc->GetPrintRangeCount(0));
    CPPUNIT_ASSERT_EQUAL(aRange1, *pDoc->GetPrintRange(0, 0));
    CPPUNIT_ASSERT_EQUAL(aRange2, *pDoc->GetPrintRange(0, 1));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf148170_ExceedXlsPrintRange)
{
    createScDoc();

    // Create print range that exceeds the xls limitations
    const auto aScSheeLimits = ScSheetLimits::CreateDefault();
    ScRange aCalcPrintRange(0, 0, 0, aScSheeLimits.MaxCol(), aScSheeLimits.MaxRow(), 0);
    ScDocument* pDoc = getScDoc();
    pDoc->AddPrintRange(0, aCalcPrintRange);

    saveAndReload(TestFilter::XLS);

    // Check if print range was shrunk to xls limitations
    pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), pDoc->GetPrintRangeCount(0));
    // Check sc/source/filter/inc/xlconst.hxx for xls limitations
    ScRange aXlsPrintRange(0, 0, 0, 16383, 65535, 0);
    CPPUNIT_ASSERT_EQUAL(aXlsPrintRange, *pDoc->GetPrintRange(0, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf138741_externalLinkSkipUnusedsCrash)
{
    createScDoc("xlsx/tdf138741_externalLinkSkipUnusedsCrash.xlsx");

    //without the fix in place, it would have crashed at export time
    save(TestFilter::XLSX);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf138824_linkToParentDirectory)
{
    createScDoc("ods/childDir/tdf138824_linkToParentDirectory.ods");

    ScDocument* pDoc = getScDoc();

    // saveAndReload save the file to a temporary directory
    // the link must be changed to point to that parent directory
    OUString aTempFilename = utl::CreateTempURL();
    auto nIdxOfTmpFile = aTempFilename.lastIndexOf('/');
    nIdxOfTmpFile = aTempFilename.lastIndexOf('/', nIdxOfTmpFile);
    aTempFilename = aTempFilename.copy(0, nIdxOfTmpFile + 1);

    // change external link to tmp directory
    OUString aFormula = pDoc->GetFormula(3, 1, 0);
    auto nIdxOfFilename = aFormula.indexOf("tdf138824_externalSource.ods");
    auto nIdxOfFile = aFormula.indexOf("file");

    aFormula = aFormula.replaceAt(nIdxOfFile, nIdxOfFilename - nIdxOfFile, aTempFilename);
    pDoc->SetFormula(ScAddress(3, 1, 0), aFormula, formula::FormulaGrammar::GRAM_NATIVE_UI);

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDocXml = parseExport(u"xl/externalLinks/_rels/externalLink1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pDocXml);

    // test also the Linux specific bug tdf#121472
    assertXPath(pDocXml, "/rels:Relationships/rels:Relationship", "Target",
                u"../tdf138824_externalSource.ods");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf129969)
{
    createScDoc("ods/external_hyperlink.ods");

    saveAndReload(TestFilter::XLSX);
    ScDocument* pDoc = getScDoc();
    ScAddress aPos(0, 0, 0);
    const EditTextObject* pEditText = pDoc->GetEditText(aPos);
    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    CPPUNIT_ASSERT(pURLData->GetURL().endsWith("/%23folder/test.ods#Sheet2.B10"));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf147088)
{
    createScDoc("fods/tdf147088.fods");

    saveAndReload(TestFilter::XLSX);

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: _xffff_
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(u"_xffff_"_ustr, pDoc->GetString(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf84874)
{
    createScDoc("ods/tdf84874.ods");

    saveAndReload(TestFilter::XLSX);

    ScDocument* pDoc = getScDoc();

    const ScValidationData* pData = pDoc->GetValidationEntry(1);
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
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf136721_paper_size)
{
    createScDoc("xlsx/tdf136721_letter_sized_paper.xlsx");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:pageSetup", "paperSize", u"70");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf139258_rotated_image)
{
    // Check that the topleft position of the image is correct.
    createScDoc("ods/tdf139258_rotated_image.ods");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pDrawing = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDrawing);

    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:from/xdr:col", u"1");
    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:from/xdr:row", u"12");
    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:to/xdr:col", u"6");
    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:to/xdr:row", u"25");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testtdf169496_hidden_graphic)
{
    createScDoc("xlsx/tdf169496_hidden_graphic.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pDrawing = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDrawing);

    // Graphic 4 is hidden and Graphic 3 is visible, but their order might change in the XML
    // Without the fix the hidden attribute wasn't exported
    OUString sName1 = getXPath(
        pDrawing, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:pic/xdr:nvPicPr/xdr:cNvPr", "name");
    OUString sName2 = getXPath(
        pDrawing, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:pic/xdr:nvPicPr/xdr:cNvPr", "name");
    if (sName1 == "Graphic 4" && sName2 == "Graphic 3")
    {
        OUString aHidden = getXPath(
            pDrawing, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:pic/xdr:nvPicPr/xdr:cNvPr", "hidden");
        bool bHidden = aHidden == u"true"_ustr || aHidden == u"1";
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Graphic 3 is supposed to be hidden", true, bHidden);

        assertXPathNoAttribute(
            pDrawing, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:pic/xdr:nvPicPr/xdr:cNvPr", "hidden");
    }
    else if (sName1 == "Graphic 3" && sName2 == "Graphic 4")
    {
        assertXPathNoAttribute(
            pDrawing, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:pic/xdr:nvPicPr/xdr:cNvPr", "hidden");

        OUString aHidden = getXPath(
            pDrawing, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:pic/xdr:nvPicPr/xdr:cNvPr", "hidden");
        bool bHidden = aHidden == u"true"_ustr || aHidden == u"1";
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Graphic 4 is supposed to be hidden", true, bHidden);
    }
    else
        CPPUNIT_FAIL("Names of graphics is incorrect");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf144642_RowHeightRounding_saveByCalc)
{
    // MS Excel round down row heights to 0.75pt
    // MS Excel can save a row height of 28.35pt, but will display it as a row height of 27.75pt.
    // Calc simulates this roundings but only if the xlsx file was saved in MS Excel.

    createScDoc("xlsx/tdf144642_RowHeight_10mm_SavedByCalc.xlsx");
    ScDocument* pDoc = getScDoc();
    // 10mm == 567 twips == 28.35pt
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(567), pDoc->GetRowHeight(0, 0));
    CPPUNIT_ASSERT_EQUAL(tools::Long(567 * 26), pDoc->GetRowHeight(0, 25, 0, true));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf144642_RowHeightRounding_saveByExcel)
{
    createScDoc("xlsx/tdf144642_RowHeight_28.35pt_SavedByExcel.xlsx");
    ScDocument* pDoc = getScDoc();
    // 555twips == 27.75pt == 9.79mm
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(555), pDoc->GetRowHeight(0, 0));
    CPPUNIT_ASSERT_EQUAL(tools::Long(555 * 26), pDoc->GetRowHeight(0, 25, 0, true));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf145129_DefaultRowHeightRounding)
{
    // MS Excel round down row heights to 0.75pt
    // Same as Tdf144642 but with default row height.

    createScDoc("xlsx/tdf145129_DefaultRowHeight_28.35pt_SavedByExcel.xlsx");
    ScDocument* pDoc = getScDoc();
    // 555twips == 27.75pt == 9.79mm
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(555), pDoc->GetRowHeight(0, 0));
    CPPUNIT_ASSERT_EQUAL(tools::Long(555 * 52), pDoc->GetRowHeight(0, 51, 0, true));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf151755_stylesLostOnXLSXExport)
{
    // Check if empty cells with custom style are exported, even if
    // there is other empty cells with default style, left of it.
    createScDoc("xlsx/tdf151755_stylesLostOnXLSXExport.xlsx");

    // Resave the xlsx file without any modification.
    save(TestFilter::XLSX);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // Check if all the 3 empty cells with styles are saved, and have the same style id.
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c", 4);
    OUString aCellStyleId = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c[2]", "s");
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c[2]", "s", aCellStyleId);
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c[3]", "s", aCellStyleId);
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c[4]", "s", aCellStyleId);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf152581_bordercolorNotExportedToXLSX)
{
    createScDoc("xlsx/tdf152581_bordercolorNotExportedToXLSX.xlsx");

    // Resave the xlsx file without any modification.
    save(TestFilter::XLSX);
    xmlDocUniquePtr pStyles = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pStyles);

    // Check if conditional format border color is exported
    assertXPath(pStyles, "/x:styleSheet/x:dxfs/x:dxf/x:border/x:left/x:color", "theme", u"5");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf140431)
{
    createScDoc("xlsx/129969-min.xlsx");

    saveAndReload(TestFilter::XLSX);
    ScDocument* pDoc = getScDoc();
    ScAddress aPos(0, 2, 0);
    const EditTextObject* pEditText = pDoc->GetEditText(aPos);
    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    CPPUNIT_ASSERT(pURLData->GetURL().startsWith("file://ndhlis"));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testCheckboxFormControlXlsxExport)
{
    if (!IsDefaultDPI())
        return;
    // Given a document that has a checkbox form control:
    createScDoc("xlsx/checkbox-form-control.xlsx");

    // When exporting to XLSX:
    saveAndReload(TestFilter::XLSX);

    // Then make sure its VML markup is written and it has a correct position + size:
    xmlDocUniquePtr pDoc = parseExport(u"xl/drawings/vmlDrawing1.vml"_ustr);
    // Without the fix in place, this test would have failed as there was no such stream.
    CPPUNIT_ASSERT(pDoc);
    assertXPathContent(pDoc, "/xml/v:shape/xx:ClientData/xx:Anchor", u"1, 22, 3, 3, 3, 30, 6, 1");

    // reloaded document: make sure it still has a flat (non-3d) look
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(0),
                                                         UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(u"Check Box 1"_ustr, sLabel);

    sal_Int16 nStyle;
    xPropertySet->getPropertyValue(u"VisualEffect"_ustr) >>= nStyle;
    // without the fix, this was 1 (3d)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nStyle); // flat

    Color aColor(COL_TRANSPARENT);
    xPropertySet->getPropertyValue(u"BackgroundColor"_ustr) >>= aColor;
    // without the fix, this was COL_WHITE
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, aColor);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testButtonFormControlXlsxExport)
{
    // Given a document that has a checkbox form control:
    createScDoc("xlsx/button-form-control.xlsx");

    // When exporting to XLSX:
    save(TestFilter::XLSX);

    // Then make sure its control markup is written and it has a correct position + size:
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    // Without the fix in place, this test would have failed with:
    // - XPath '//x:anchor/x:from/xdr:col' not found
    // i.e. the control markup was missing, the button was lost on export.
    assertXPathContent(pDoc, "//x:anchor/x:from/xdr:col", u"1");
    assertXPathContent(pDoc, "//x:anchor/x:from/xdr:row", u"3");
    assertXPathContent(pDoc, "//x:anchor/x:to/xdr:col", u"3");
    assertXPathContent(pDoc, "//x:anchor/x:to/xdr:row", u"7");

    // Also make sure that an empty macro attribute is not written.
    // Without the fix in place, this test would have failed with:
    // - XPath '//x:controlPr' unexpected 'macro' attribute
    // i.e. macro in an xlsx file was not omitted, which is considered invalid by Excel.
    assertXPathNoAttribute(pDoc, "//x:controlPr", "macro");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
