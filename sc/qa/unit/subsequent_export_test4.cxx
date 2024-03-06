/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <officecfg/Office/Common.hxx>

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <docpool.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <stlpool.hxx>
#include <formulacell.hxx>
#include <postit.hxx>
#include <validat.hxx>
#include <scresid.hxx>
#include <globstr.hrc>

#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/justifyitem.hxx>
#include <comphelper/scopeguard.hxx>
#include <formula/grammar.hxx>
#include <tools/fldunit.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <svx/svdocapt.hxx>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/sheet/GlobalSheetSettings.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XTextColumns.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScExportTest4 : public ScModelTestBase
{
public:
    ScExportTest4();
};

ScExportTest4::ScExportTest4()
    : ScModelTestBase("sc/qa/unit/data")
{
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf126177XLSX)
{
    createScDoc("xlsx/hyperlink_export.xlsx");
    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink"_ostr, "location"_ostr, "Munka1!A5");

    xmlDocUniquePtr pXmlRels = parseExport("xl/worksheets/_rels/sheet1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    OUString aTarget
        = getXPath(pXmlRels, "/rels:Relationships/rels:Relationship"_ostr, "Target"_ostr);
    CPPUNIT_ASSERT(aTarget.endsWith("test.xlsx"));
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship"_ostr, "TargetMode"_ostr,
                "External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testCommentTextVAlignment)
{
    // Testing comment text alignments.
    createScDoc("ods/CommentTextVAlign.ods");

    save("Calc Office Open XML");

    xmlDocUniquePtr pVmlDrawing = parseExport("xl/drawings/vmlDrawing1.vml");
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/xml/v:shape/xx:ClientData/xx:TextVAlign"_ostr, "Center");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testCommentTextHAlignment)
{
    // Testing comment text alignments.
    createScDoc("ods/CommentTextHAlign.ods");

    save("Calc Office Open XML");

    xmlDocUniquePtr pVmlDrawing = parseExport("xl/drawings/vmlDrawing1.vml");
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/xml/v:shape/xx:ClientData/xx:TextHAlign"_ostr, "Center");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testRotatedImageODS)
{
    // Error was, that the length values in shapes were not
    // written in the given unit into the file.
    css::uno::Reference<css::sheet::XGlobalSheetSettings> xGlobalSheetSettings
        = css::sheet::GlobalSheetSettings::create(comphelper::getProcessComponentContext());
    xGlobalSheetSettings->setMetric(static_cast<sal_Int16>(FieldUnit::MM));

    createScDoc("ods/tdf103092_RotatedImage.ods");

    save("calc8");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    const OUString sTransform = getXPath(pXmlDoc,
                                         "/office:document-content/office:body/office:spreadsheet/"
                                         "table:table/table:shapes/draw:frame"_ostr,
                                         "transform"_ostr);
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

    save("calc8");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // no attribute 'value' exist
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table/office:forms/"
                "form:form/form:radio[1]"_ostr,
                "value"_ostr, "1");
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table/office:forms/"
                "form:form/form:radio[2]"_ostr,
                "value"_ostr, "2");
    const OUString sGroupName1 = getXPath(pXmlDoc,
                                          "/office:document-content/office:body/office:spreadsheet/"
                                          "table:table/office:forms/form:form/form:radio[1]"_ostr,
                                          "group-name"_ostr);
    const OUString sGroupName2 = getXPath(pXmlDoc,
                                          "/office:document-content/office:body/office:spreadsheet/"
                                          "table:table/office:forms/form:form/form:radio[2]"_ostr,
                                          "group-name"_ostr);
    CPPUNIT_ASSERT_EQUAL(sGroupName1, sGroupName2);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf85553)
{
    createScDoc("ods/tdf85553.ods");

    saveAndReload("MS Excel 97");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 4.5
    // - Actual  : #N/A
    CPPUNIT_ASSERT_EQUAL(OUString("4.5"), pDoc->GetString(ScAddress(2, 2, 0)));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf151484)
{
    std::vector<OUString> aFilterNames{ "calc8", "MS Excel 97", "Calc Office Open XML" };

    for (size_t i = 0; i < aFilterNames.size(); ++i)
    {
        createScDoc("ods/tdf151484.ods");

        const OString sFailedMessage
            = OString::Concat("Failed on filter: ") + aFilterNames[i].toUtf8();

        saveAndReload(aFilterNames[i]);

        ScDocument* pDoc = getScDoc();

        const ScValidationData* pData = pDoc->GetValidationEntry(1);
        CPPUNIT_ASSERT(pData);

        std::vector<ScTypedStrData> aList;
        pData->FillSelectionList(aList, ScAddress(0, 1, 0));

        // Without the fix in place, this test would have failed with
        // - Expected: 4
        // - Actual  : 1
        // - Failed on filter: MS Excel 97
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), size_t(4), aList.size());
        for (size_t j = 0; j < 4; ++j)
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), double(j + 1),
                                         aList[j].GetValue());
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf143979)
{
    createScDoc();
    {
        ScDocument* pDoc = getScDoc();
        OUString aCode = "YYYY-MM\"\"MMM-DDNN";
        sal_Int32 nCheckPos;
        SvNumFormatType nType;
        sal_uInt32 nFormat;
        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        pFormatter->PutEntry(aCode, nCheckPos, nType, nFormat);
        ScPatternAttr aNewAttrs(pDoc->getCellAttributeHelper());
        SfxItemSet& rSet = aNewAttrs.GetItemSet();
        rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));
        pDoc->ApplyPattern(0, 0, 0, aNewAttrs);
        pDoc->SetString(ScAddress(0, 0, 0), "08/30/2021");
        CPPUNIT_ASSERT_EQUAL(OUString("2021-08Aug-30Mon"), pDoc->GetString(ScAddress(0, 0, 0)));
    }

    saveAndReload("calc8");

    ScDocument* pDoc = getScDoc();
    // Without the fix in place, this test would have failed with
    // - Expected: 2021-08Aug-30Mon
    // - Actual  : 2021-A-30Mon
    CPPUNIT_ASSERT_EQUAL(OUString("2021-08Aug-30Mon"), pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf128976)
{
    createScDoc("xls/tdf128976.xls");

    saveAndReload("MS Excel 97");

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

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet1 = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet1);

    // This was 1025 when nMaxCol+1 was 1024
    assertXPath(pSheet1, "/x:worksheet/x:cols/x:col"_ostr, "max"_ostr,
                OUString::number(nMaxCol + 1));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf131372)
{
    createScDoc("ods/tdf131372.ods");

    save("Calc Office Open XML");

    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c[1]/x:f"_ostr, "NA()");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c[2]/x:f"_ostr, "#N/A");
}
CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf81470)
{
    createScDoc("xls/tdf81470.xls");

    //without the fix in place, it would have crashed at export time
    save("Calc Office Open XML");

    //also check revisions are exported
    xmlDocUniquePtr pHeaders = parseExport("xl/revisions/revisionHeaders.xml");
    CPPUNIT_ASSERT(pHeaders);

    assertXPath(pHeaders, "/x:headers/x:header[1]"_ostr, "dateTime"_ostr,
                "2014-07-11T13:46:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[1]"_ostr, "userName"_ostr, "Kohei Yoshida");
    assertXPath(pHeaders, "/x:headers/x:header[2]"_ostr, "dateTime"_ostr,
                "2014-07-11T18:38:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[2]"_ostr, "userName"_ostr, "Kohei Yoshida");
    assertXPath(pHeaders, "/x:headers/x:header[3]"_ostr, "dateTime"_ostr,
                "2014-07-11T18:43:00.000000000Z");
    assertXPath(pHeaders, "/x:headers/x:header[3]"_ostr, "userName"_ostr, "Kohei Yoshida");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf122331)
{
    createScDoc("ods/tdf122331.ods");

    save("Calc Office Open XML");

    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:sheetPr"_ostr, "filterMode"_ostr, "true");
    assertXPath(pSheet, "/x:worksheet/x:autoFilter"_ostr, "ref"_ostr, "A1:B761");
    assertXPath(pSheet, "/x:worksheet/x:autoFilter/x:filterColumn"_ostr, "colId"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf83779)
{
    // Roundtripping TRUE/FALSE constants (not functions) must convert them to functions
    createScDoc("xlsx/tdf83779.xlsx");

    save("Calc Office Open XML");

    xmlDocUniquePtr pVmlDrawing = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pVmlDrawing);

    assertXPathContent(pVmlDrawing, "/x:worksheet/x:sheetData/x:row[1]/x:c/x:f"_ostr, "FALSE()");
    assertXPathContent(pVmlDrawing, "/x:worksheet/x:sheetData/x:row[2]/x:c/x:f"_ostr, "TRUE()");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf121715_FirstPageHeaderFooterXLSX)
{
    // Check if first page header and footer are exported properly
    createScDoc("xlsx/tdf121715.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:headerFooter"_ostr, "differentFirst"_ostr, "true");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:firstHeader"_ostr,
                       "&CFirst Page Header");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:firstFooter"_ostr,
                       "&CFirst Page Footer");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf121716_ExportEvenHeaderFooterXLSX)
{
    // Header and footer on even pages should be exported properly
    // If there are separate odd/even header, but only 1 footer for all pages (this is possible only in LibreOffice)
    //  then the footer will be duplicated to have the same footer separately for even/odd pages

    createScDoc("ods/tdf121716_EvenHeaderFooter.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:headerFooter"_ostr, "differentOddEven"_ostr, "true");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader"_ostr,
                       "&Lodd/right&Cpage&Rheader");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter"_ostr,
                       "&Lboth&C&12page&Rfooter");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenHeader"_ostr,
                       "&Lpage&Cheader&Reven/left");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenFooter"_ostr,
                       "&Lboth&C&12page&Rfooter");

    pDoc = parseExport("xl/worksheets/sheet2.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:headerFooter"_ostr, "differentOddEven"_ostr, "true");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader"_ostr, "&Coddh");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter"_ostr, "&Coddf");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenHeader"_ostr, "&Cevenh");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:evenFooter"_ostr, "&Levenf");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf134459_HeaderFooterColorXLSX)
{
    // Colors in header and footer should be exported, and imported properly
    createScDoc("xlsx/tdf134459_HeaderFooterColor.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader"_ostr,
                       "&L&Kc06040l&C&K4c3789c&Rr");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter"_ostr,
                       "&Ll&C&K64cf5fc&R&Kcd15aar");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf134817_HeaderFooterTextWith2SectionXLSX)
{
    // Header/footer text with multiple selection should be exported, and imported properly
    createScDoc("xlsx/tdf134817_HeaderFooterTextWith2Section.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddHeader"_ostr,
                       "&L&\"Abadi,Regular\"&11aaa&\"Bembo,Regular\"&20bbb");
    assertXPathContent(pDoc, "/x:worksheet/x:headerFooter/x:oddFooter"_ostr,
                       "&R&\"Cambria,Regular\"&14camb&\"Dante,Regular\"&18dant");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf121718_UseFirstPageNumberXLSX)
{
    // If "First page number" is not checked then useFirstPageNumb, and firstPageNumber should not be exported.
    createScDoc("ods/tdf121718_UseFirstPageNumber.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:pageSetup"_ostr, "useFirstPageNumber"_ostr, "true");
    assertXPath(pDoc, "/x:worksheet/x:pageSetup"_ostr, "firstPageNumber"_ostr, "10");

    pDoc = parseExport("xl/worksheets/sheet2.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPathNoAttribute(pDoc, "/x:worksheet/x:pageSetup"_ostr, "useFirstPageNumber"_ostr);
    assertXPathNoAttribute(pDoc, "/x:worksheet/x:pageSetup"_ostr, "firstPageNumber"_ostr);
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
    constexpr OUString aTestPageStyle = u"TestPageStyle"_ustr;
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(aTestPageStyle, pDoc->GetPageStyle(0));

    // Change page style to default so the user defined one is not used anymore
    pDoc->SetPageStyle(0, ScResId(STR_STYLENAME_STANDARD));

    // Save and reload the document to check if the unused page styles are still present
    saveAndReload("calc8");
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

    save("Calc Office Open XML");

    xmlDocUniquePtr pDrawing = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    double nXPosOfTopleft
        = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:off"_ostr,
                   "x"_ostr)
              .toDouble();
    double nYPosOfTopleft
        = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:off"_ostr,
                   "y"_ostr)
              .toDouble();
    double nWidth
        = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:ext"_ostr,
                   "cx"_ostr)
              .toDouble();
    double nHeight
        = getXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:spPr/a:xfrm/a:ext"_ostr,
                   "cy"_ostr)
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
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_HOR_JUSTIFY);
        const SvxHorJustifyItem& rJustify = static_cast<const SvxHorJustifyItem&>(rItem);
        CPPUNIT_ASSERT_EQUAL(SvxCellHorJustify::Center, rJustify.GetValue());
    }

    {
        const ScProtectionAttr& rItem = pAttr->GetItem(ATTR_PROTECTION);
        CPPUNIT_ASSERT(rItem.GetProtection());
        CPPUNIT_ASSERT(!rItem.GetHideFormula());
    }

    pAttr = pDoc->GetPattern(2, 0, 0); //C1

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

    pAttr = pDoc->GetPattern(2, 1, 0); //C2

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
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf123353)
{
    createScDoc("xlsx/tdf123353.xlsx");

    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:autoFilter/x:filterColumn/x:filters"_ostr, "blank"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf140098)
{
    createScDoc("ods/tdf140098.ods");

    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:autoFilter/x:filterColumn/x:filters"_ostr, "blank"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf133688_precedents)
{
    // tdf#133688 Check that we do not export detective shapes.
    createScDoc("ods/tdf133688_dont_save_precedents_to_xlsx.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDrawing = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    // We do not export any shapes.
    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor[1]"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf91251_missingOverflowRoundtrip)
{
    // tdf#91251 check whether textBox overflow property (horzOverflow and vertOverflow) is
    // getting preserved after roundtrip
    createScDoc("xlsx/tdf91251_missingOverflowRoundtrip.xlsx");

    save("Calc Office Open XML");

    xmlDocUniquePtr pDrawing = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr"_ostr,
                "horzOverflow"_ostr, "clip");
    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr"_ostr,
                "horzOverflow"_ostr, "clip");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf137000_handle_upright)
{
    // Upright is an xml attribute of xdr:txBody/a:bodyPr. It is set when in a textbox menu we
    // choose, 'do not rotate this element'. Implementations are in tdf#106197 with followup
    // tdf#137000. tdf#149538, tdf#149551 improve the implementation to export 'upright' instead
    // of workaround 'rot'.
    createScDoc("xlsx/tdf137000_export_upright.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDrawing = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:txBody/a:bodyPr"_ostr,
                "upright"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf126305_DataValidatyErrorAlert)
{
    createScDoc("ods/tdf126305.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[1]"_ostr, "errorStyle"_ostr,
                "stop");
    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[2]"_ostr, "errorStyle"_ostr,
                "warning");
    assertXPath(pDoc, "/x:worksheet/x:dataValidations/x:dataValidation[3]"_ostr, "errorStyle"_ostr,
                "information");
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
    saveAndReload("Calc Office Open XML");

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
    saveAndReload("calc8");

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

    saveAndReload("MS Excel 97");

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
    save("Calc Office Open XML");
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

    save("Calc Office Open XML");
    xmlDocUniquePtr pDocXml = parseExport("xl/externalLinks/_rels/externalLink1.xml.rels");
    CPPUNIT_ASSERT(pDocXml);

    // test also the Linux specific bug tdf#121472
    assertXPath(pDocXml, "/rels:Relationships/rels:Relationship"_ostr, "Target"_ostr,
                "../tdf138824_externalSource.ods");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf129969)
{
    createScDoc("ods/external_hyperlink.ods");

    saveAndReload("Calc Office Open XML");
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

    saveAndReload("Calc Office Open XML");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: _xffff_
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("_xffff_"), pDoc->GetString(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf84874)
{
    createScDoc("ods/tdf84874.ods");

    saveAndReload("Calc Office Open XML");

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

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:worksheet/x:pageSetup"_ostr, "paperSize"_ostr, "70");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf139258_rotated_image)
{
    // Check that the topleft position of the image is correct.
    createScDoc("ods/tdf139258_rotated_image.ods");

    save("Calc Office Open XML");

    xmlDocUniquePtr pDrawing = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:from/xdr:col"_ostr, "1");
    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:from/xdr:row"_ostr, "12");
    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:to/xdr:col"_ostr, "6");
    assertXPathContent(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor/xdr:to/xdr:row"_ostr, "25");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf144642_RowHeightRounding)
{
    // MS Excel round down row heights to 0.75pt
    // MS Excel can save a row height of 28.35pt, but will display it as a row height of 27.75pt.
    // Calc simulates this roundings but only if the xlsx file was saved in MS Excel.

    createScDoc("xlsx/tdf144642_RowHeight_10mm_SavedByCalc.xlsx");
    ScDocument* pDoc = getScDoc();
    // 10mm == 567 twips == 28.35pt
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(567), pDoc->GetRowHeight(0, 0));
    CPPUNIT_ASSERT_EQUAL(tools::Long(567 * 26), pDoc->GetRowHeight(0, 25, 0, true));

    createScDoc("xlsx/tdf144642_RowHeight_28.35pt_SavedByExcel.xlsx");
    pDoc = getScDoc();
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
    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // Check if all the 3 empty cells with styles are saved, and have the same style id.
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c"_ostr, 4);
    OUString aCellStyleId
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c[2]"_ostr, "s"_ostr);
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c[2]"_ostr, "s"_ostr, aCellStyleId);
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c[3]"_ostr, "s"_ostr, aCellStyleId);
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c[4]"_ostr, "s"_ostr, aCellStyleId);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf152581_bordercolorNotExportedToXLSX)
{
    createScDoc("xlsx/tdf152581_bordercolorNotExportedToXLSX.xlsx");

    // Resave the xlsx file without any modification.
    save("Calc Office Open XML");
    xmlDocUniquePtr pStyles = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pStyles);

    // Check if conditional format border color is exported
    assertXPath(pStyles, "/x:styleSheet/x:dxfs/x:dxf/x:border/x:left/x:color"_ostr, "theme"_ostr,
                "5");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf140431)
{
    createScDoc("xlsx/129969-min.xlsx");

    saveAndReload("Calc Office Open XML");
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
    save("Calc Office Open XML");

    // Then make sure its VML markup is written and it has a correct position + size:
    xmlDocUniquePtr pDoc = parseExport("xl/drawings/vmlDrawing1.vml");
    // Without the fix in place, this test would have failed as there was no such stream.
    CPPUNIT_ASSERT(pDoc);
    assertXPathContent(pDoc, "/xml/v:shape/xx:ClientData/xx:Anchor"_ostr,
                       "1, 22, 3, 3, 3, 30, 6, 1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testButtonFormControlXlsxExport)
{
    // Given a document that has a checkbox form control:
    createScDoc("xlsx/button-form-control.xlsx");

    // When exporting to XLSX:
    save("Calc Office Open XML");

    // Then make sure its control markup is written and it has a correct position + size:
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    // Without the fix in place, this test would have failed with:
    // - XPath '//x:anchor/x:from/xdr:col' not found
    // i.e. the control markup was missing, the button was lost on export.
    assertXPathContent(pDoc, "//x:anchor/x:from/xdr:col"_ostr, "1");
    assertXPathContent(pDoc, "//x:anchor/x:from/xdr:row"_ostr, "3");
    assertXPathContent(pDoc, "//x:anchor/x:to/xdr:col"_ostr, "3");
    assertXPathContent(pDoc, "//x:anchor/x:to/xdr:row"_ostr, "7");

    // Also make sure that an empty macro attribute is not written.
    // Without the fix in place, this test would have failed with:
    // - XPath '//x:controlPr' unexpected 'macro' attribute
    // i.e. macro in an xlsx file was not omitted, which is considered invalid by Excel.
    assertXPathNoAttribute(pDoc, "//x:controlPr"_ostr, "macro"_ostr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf142929_filterLessThanXLSX)
{
    // Document contains a standard filter with '<' condition.
    createScDoc("xlsx/tdf142929.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "//x:customFilters/x:customFilter"_ostr, "val"_ostr, "2");
    assertXPath(pDoc, "//x:customFilters/x:customFilter"_ostr, "operator"_ostr, "lessThan");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testInvalidNamedRange)
{
    // Given a document which has a named range (myname) that refers to the "1" external link, but
    // the link's type is xlPathMissing, when importing that document:
    createScDoc("xlsx/invalid-named-range.xlsx");

    // Then make sure that named range is ignored, as "1" can't be resolved, and exporting it back
    // to XLSX (without the xlPathMissing link) would corrupt the document:
    uno::Reference<beans::XPropertySet> xDocProps(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNamedRanges(xDocProps->getPropertyValue("NamedRanges"),
                                                        uno::UNO_QUERY);
    // Without the fix in place, this test would have failed, we didn't ignore the problematic named
    // range on import.
    CPPUNIT_ASSERT(!xNamedRanges->hasByName("myname"));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testExternalDefinedNameXLSX)
{
    createScDoc("xlsx/tdf144397.xlsx");
    saveAndReload("Calc Office Open XML");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->ReloadAllLinks();
    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    // "January"
    {
        const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 1, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::String, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(OUString("January"), aRes.maString.getString());
    }
    // "March"
    {
        const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 3, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::String, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(OUString("March"), aRes.maString.getString());
    }
    // "Empty = #N/A"
    {
        const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 5, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::Error, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(OUString(""), aRes.maString.getString());
    }
    // "June"
    {
        const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 6, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::String, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(OUString("June"), aRes.maString.getString());
    }

    save("Calc Office Open XML");
    xmlDocUniquePtr pDocXml = parseExport("xl/externalLinks/externalLink1.xml");

    CPPUNIT_ASSERT(pDocXml);
    assertXPath(pDocXml, "/x:externalLink/x:externalBook/x:sheetNames/x:sheetName"_ostr, "val"_ostr,
                "Munka1");
    assertXPath(pDocXml, "/x:externalLink/x:externalBook/x:definedNames/x:definedName"_ostr,
                "name"_ostr, "MonthNames");
    // TODO: no need for the [1] external document identifier
    assertXPath(pDocXml, "/x:externalLink/x:externalBook/x:definedNames/x:definedName"_ostr,
                "refersTo"_ostr, "[1]Munka1!$A$2:$A$13");
    assertXPath(pDocXml, "/x:externalLink/x:externalBook/x:sheetDataSet/x:sheetData"_ostr,
                "sheetId"_ostr, "0");
    assertXPath(pDocXml, "/x:externalLink/x:externalBook/x:sheetDataSet/x:sheetData/x:row[2]"_ostr,
                "r"_ostr, "3");
    assertXPathContent(
        pDocXml,
        "/x:externalLink/x:externalBook/x:sheetDataSet/x:sheetData/x:row[2]/x:cell/x:v"_ostr,
        "February");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testHyperlinkLocationXLSX)
{
    createScDoc("ods/tdf143220.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    // tdf#143220 link to sheet not valid without cell reference
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[@ref='A1']"_ostr, "location"_ostr,
                "Sheet2!A1");

    // tdf#145079 link with defined name target didn't work because Calc added "A1" at the end
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[@ref='A2']"_ostr, "location"_ostr,
                "name");
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[@ref='A3']"_ostr, "location"_ostr,
                "db");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf142264ManyChartsToXLSX)
{
    // The cache size for the test should be small enough, to make sure that some charts get
    // unloaded in the process, and then loaded on demand properly (default is currently 200)
    comphelper::ScopeGuard g([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Cache::DrawingEngine::OLE_Objects::set(200, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::DrawingEngine::OLE_Objects::set(20, pBatch);
    pBatch->commit();

    createScDoc("ods/many_charts.ods");
    saveAndReload("Calc Office Open XML");

    css::uno::Reference<css::drawing::XDrawPagesSupplier> xSupplier(mxComponent,
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
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf143929MultiColumnToODS)
{
    createScDoc("ods/two-col-shape.ods");

    {
        css::uno::Reference<css::drawing::XDrawPagesSupplier> xSupplier(mxComponent,
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

    saveAndReload("calc8");
    {
        css::uno::Reference<css::drawing::XDrawPagesSupplier> xSupplier(mxComponent,
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

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the fix in place, this would have failed with:
    //   - Expected: 1
    //   - Actual  : 0
    //   - In <>, XPath '/office:document-content/office:automatic-styles/style:style[@style:family='graphic']/
    //     style:graphic-properties/style:columns' number of nodes is incorrect
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[@style:family='graphic']/"
        "style:graphic-properties/style:columns"_ostr,
        "column-count"_ostr, "2");
    // Only test that "column-gap" attribute exists, not its value that depends on locale (cm, in)
    getXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[@style:family='graphic']/"
        "style:graphic-properties/style:columns"_ostr,
        "column-gap"_ostr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf142578)
{
    createScDoc("ods/tdf142578.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // Get DxfId for color filter
    sal_Int32 nDxfIdColorFilter
        = getXPath(pSheet, "/x:worksheet/x:autoFilter/x:filterColumn/x:colorFilter"_ostr,
                   "dxfId"_ostr)
              .toInt32()
          + 1;

    // Get DxfId for conditional formatting
    sal_Int32 nDxfIdCondFormat
        = getXPath(pSheet, "/x:worksheet/x:conditionalFormatting/x:cfRule"_ostr, "dxfId"_ostr)
              .toInt32()
          + 1;

    // Ensure they are using different dxfs
    CPPUNIT_ASSERT_MESSAGE("dxfID's should be different!", nDxfIdColorFilter != nDxfIdCondFormat);

    // Check colors used by these dxfs
    xmlDocUniquePtr pStyles = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pStyles);

    OString sDxfColorFilterXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfIdColorFilter)
                                 + "]/x:fill/x:patternFill/x:fgColor");
    assertXPath(pStyles, sDxfColorFilterXPath, "rgb"_ostr, "FF81D41A");

    OString sDxfCondFormatXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfIdCondFormat)
                                + "]/x:fill/x:patternFill/x:bgColor");
    assertXPath(pStyles, sDxfCondFormatXPath, "rgb"_ostr, "FFFFCCCC");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf145059)
{
    createScDoc("ods/tdf145059.ods");

    // Export to xlsx.
    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);
    xmlDocUniquePtr pStyle = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pStyle);

    sal_Int32 nColorFilterDxdId
        = getXPath(pSheet, "/x:worksheet/x:autoFilter/x:filterColumn/x:colorFilter"_ostr,
                   "dxfId"_ostr)
              .toInt32();

    // Ensure that dxf id is not -1
    CPPUNIT_ASSERT(nColorFilterDxdId >= 0);

    // Find color by this dxfid
    OString sDxfIdPath = "/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nColorFilterDxdId + 1)
                         + "]/x:fill/x:patternFill/x:fgColor";
    assertXPath(pStyle, sDxfIdPath, "rgb"_ostr, "FF4472C4");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf130104_XLSXIndent)
{
    createScDoc("xlsx/tdf130104_indent.xlsx");

    // Resave the xlsx file without any modification.
    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);
    xmlDocUniquePtr pStyle = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pStyle);

    // Check to see whether the indents remain the same as the original ones:

    // Get the style index number for cell A1
    sal_Int32 nCellA1StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]/x:c[1]"_ostr, "s"_ostr).toInt32() + 1;
    // The indent for cell A1 should be 0
    OString sStyleA1XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA1StyleIndex) + "]/x:alignment";
    // (if this assertion fails, you should first check whether there is no style index set for this cell)
    assertXPath(pStyle, sStyleA1XPath, "indent"_ostr, "0");

    sal_Int32 nCellA3StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]/x:c[1]"_ostr, "s"_ostr).toInt32() + 1;
    // The indent for cell A3 should be 1
    OString sStyleA3XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA3StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA3XPath, "indent"_ostr, "1");

    sal_Int32 nCellA6StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]/x:c[1]"_ostr, "s"_ostr).toInt32() + 1;
    OString sStyleA6XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA6StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA6XPath, "indent"_ostr, "2");

    sal_Int32 nCellA9StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[9]/x:c[1]"_ostr, "s"_ostr).toInt32() + 1;
    OString sStyleA9XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA9StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA9XPath, "indent"_ostr, "3");

    sal_Int32 nCellA12StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[12]/x:c[1]"_ostr, "s"_ostr).toInt32()
          + 1;
    OString sStyleA12XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA12StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA12XPath, "indent"_ostr, "4");

    sal_Int32 nCellA15StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[15]/x:c[1]"_ostr, "s"_ostr).toInt32()
          + 1;
    OString sStyleA15XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA15StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA15XPath, "indent"_ostr, "5");

    sal_Int32 nCellA18StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[18]/x:c[1]"_ostr, "s"_ostr).toInt32()
          + 1;
    OString sStyleA18XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA18StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA18XPath, "indent"_ostr, "6");

    sal_Int32 nCellA21StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]/x:c[1]"_ostr, "s"_ostr).toInt32()
          + 1;
    OString sStyleA21XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA21StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA21XPath, "indent"_ostr, "7");

    sal_Int32 nCellA24StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[24]/x:c[1]"_ostr, "s"_ostr).toInt32()
          + 1;
    OString sStyleA24XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA24StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA24XPath, "indent"_ostr, "8");

    sal_Int32 nCellA27StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[27]/x:c[1]"_ostr, "s"_ostr).toInt32()
          + 1;
    OString sStyleA27XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA27StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA27XPath, "indent"_ostr, "9");

    sal_Int32 nCellA30StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[30]/x:c[1]"_ostr, "s"_ostr).toInt32()
          + 1;
    OString sStyleA30XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA30StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA30XPath, "indent"_ostr, "10");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testWholeRowBold)
{
    createScDoc();

    {
        ScDocument* pDoc = getScDoc();

        // Make entire second row bold.
        ScPatternAttr boldAttr(pDoc->getCellAttributeHelper());
        boldAttr.GetItemSet().Put(SvxWeightItem(WEIGHT_BOLD, ATTR_FONT_WEIGHT));
        pDoc->ApplyPatternAreaTab(0, 1, pDoc->MaxCol(), 1, 0, boldAttr);
    }

    saveAndReload("calc8");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(SCCOL(INITIALCOLCOUNT), pDoc->GetAllocatedColumnsCount(0));
    vcl::Font aFont;
    pDoc->GetPattern(pDoc->MaxCol(), 1, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());

    saveAndReload("Calc Office Open XML");
    pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(SCCOL(INITIALCOLCOUNT), pDoc->GetAllocatedColumnsCount(0));
    pDoc->GetPattern(pDoc->MaxCol(), 1, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testXlsxRowsOrder)
{
    createScDoc("xlsx/tdf58243.xlsx");
    // Make sure code in SheetDataBuffer doesn't assert columns/rows sorting.
    save("Calc Office Open XML");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf91286)
{
    createScDoc("ods/tdf91286.ods");
    save("Calc Office Open XML");

    Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    const Sequence<OUString> aNames(xNameAccess->getElementNames());
    int nImageFiles = 0;
    for (const auto& rElementName : aNames)
        if (rElementName.startsWith("xl/media/image"))
            nImageFiles++;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the embedded picture would have been saved twice.
    CPPUNIT_ASSERT_EQUAL(1, nImageFiles);
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf148820)
{
    createScDoc("xlsx/tdf148820.xlsx");
    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    sal_Int32 nDxfIdCondFormatFirst
        = getXPath(pSheet, "/x:worksheet/x:conditionalFormatting[1]/x:cfRule"_ostr, "dxfId"_ostr)
              .toInt32()
          + 1;
    sal_Int32 nDxfIdCondFormatLast
        = getXPath(pSheet, "/x:worksheet/x:conditionalFormatting[20]/x:cfRule"_ostr, "dxfId"_ostr)
              .toInt32()
          + 1;

    xmlDocUniquePtr pStyles = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pStyles);

    OString sDxfCondFormatXPath("/x:styleSheet/x:dxfs/x:dxf["
                                + OString::number(nDxfIdCondFormatFirst)
                                + "]/x:fill/x:patternFill/x:bgColor");
    assertXPath(pStyles, sDxfCondFormatXPath, "rgb"_ostr, "FF53B5A9");
    sDxfCondFormatXPath
        = OString("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfIdCondFormatLast)
                  + "]/x:fill/x:patternFill/x:bgColor");
    assertXPath(pStyles, sDxfCondFormatXPath, "rgb"_ostr, "FFA30000");
}

namespace
{
void lcl_TestNumberFormat(ScDocument& rDoc, const OUString& rFormatStrOK)
{
    sal_uInt32 nNumberFormat = rDoc.GetNumberFormat(0, 0, 0);
    const SvNumberformat* pNumberFormat = rDoc.GetFormatTable()->GetEntry(nNumberFormat);
    const OUString& rFormatStr = pNumberFormat->GetFormatstring();

    CPPUNIT_ASSERT_EQUAL(rFormatStrOK, rFormatStr);
}

void lcl_SetNumberFormat(ScDocument& rDoc, const OUString& rFormat)
{
    sal_Int32 nCheckPos;
    SvNumFormatType nType;
    sal_uInt32 nFormat;
    OUString aNewFormat = rFormat;
    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    if (pFormatter)
    {
        pFormatter->PutEntry(aNewFormat, nCheckPos, nType, nFormat);
        rDoc.SetNumberFormat(ScAddress(0, 0, 0), nFormat);
    }
}
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testBlankInExponent)
{
    createScDoc("ods/tdf156449-Blank-In-Exponent.ods");

    // save to ODS and reload
    saveAndReload("calc8");
    lcl_TestNumberFormat(*getScDoc(), "0.00E+?0");
    lcl_SetNumberFormat(*getScDoc(), "0.00E+??");
    // at least one '0' in exponent
    saveAndReload("calc8");
    lcl_TestNumberFormat(*getScDoc(), "0.00E+?0");

    // save to XLSX and reload
    saveAndReload("Calc Office Open XML");
    lcl_TestNumberFormat(*getScDoc(), "0.00E+?0");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testSecondsWithoutTruncateAndDecimals)
{
    createScDoc("xlsx/seconds-without-truncate-and-decimals.xlsx");
    lcl_TestNumberFormat(*getScDoc(), "[SS].00");

    // save to ODS and reload
    saveAndReload("calc8");
    lcl_TestNumberFormat(*getScDoc(), "[SS].00");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testBlankWidthCharacter)
{
    createScDoc("ods/tdf152724-Blank-width-char.ods");

    // save to ODS and reload
    saveAndReload("calc8");
    lcl_TestNumberFormat(*getScDoc(), "[>0]_-?0;[<0]-?0;_-?0;@");

    // save to XLSX and reload
    saveAndReload("Calc Office Open XML");
    lcl_TestNumberFormat(*getScDoc(), "_-?0;-?0;_-?0;@");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testEmbeddedTextInDecimal)
{
    createScDoc("xlsx/embedded-text-in-decimal.xlsx");
    lcl_TestNumberFormat(*getScDoc(), "#,##0.000\" \"###\" \"###");

    // save to ODS and reload
    saveAndReload("calc8");
    lcl_TestNumberFormat(*getScDoc(), "#,##0.000\" \"###\" \"###");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testLowercaseExponent)
{
    createScDoc("ods/tdf153993-Exponent-lower-case.ods");

    // save to ODS and reload
    saveAndReload("calc8");
    lcl_TestNumberFormat(*getScDoc(), "0.000\" \"000\" \"e+\" \"0");

    // save to XLSX and reload
    // lower case not preserve in XLSX
    saveAndReload("Calc Office Open XML");
    lcl_TestNumberFormat(*getScDoc(), "0.000 000 E+ 0");
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTotalsRowFunction)
{
    createScDoc("xlsx/totalsRowFunction.xlsx");
    saveAndReload("Calc Office Open XML");
    {
        xmlDocUniquePtr pDocXml = parseExport("xl/tables/table1.xml");
        CPPUNIT_ASSERT(pDocXml);
        assertXPath(pDocXml, "/x:table/x:tableColumns/x:tableColumn[5]"_ostr,
                    "totalsRowFunction"_ostr, "sum");
    }
    ScDocument* pDoc = getScDoc();
    pDoc->InsertCol(ScRange(3, 0, 0, 3, pDoc->MaxRow(), 0)); // Insert col 4
    saveAndReload("Calc Office Open XML");
    {
        xmlDocUniquePtr pDocXml = parseExport("xl/tables/table1.xml");
        CPPUNIT_ASSERT(pDocXml);
        assertXPathNoAttribute(pDocXml, "/x:table/x:tableColumns/x:tableColumn[5]"_ostr,
                               "totalsRowFunction"_ostr);
        assertXPath(pDocXml, "/x:table/x:tableColumns/x:tableColumn[6]"_ostr,
                    "totalsRowFunction"_ostr, "sum");
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testAutofilterHiddenButton)
{
    createScDoc("xlsx/hiddenButton.xlsx");
    saveAndReload("Calc Office Open XML");
    xmlDocUniquePtr pDocXml = parseExport("xl/tables/table1.xml");
    CPPUNIT_ASSERT(pDocXml);
    for (int i = 1; i <= 5; i++)
    {
        OString sPath
            = OString::Concat("/x:table/x:autoFilter/x:filterColumn[") + OString::number(i) + "]";
        assertXPath(pDocXml, sPath, "hiddenButton"_ostr, "1");
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testShapeStyles)
{
    createScDoc();

    {
        uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                             uno::UNO_QUERY_THROW);
        uno::Reference<container::XNameContainer> xGraphicStyles(
            xStyleFamiliesSupplier->getStyleFamilies()->getByName("GraphicStyles"),
            uno::UNO_QUERY_THROW);

        // create styles
        uno::Reference<style::XStyle> xStyle(
            xMSF->createInstance("com.sun.star.style.GraphicStyle"), uno::UNO_QUERY_THROW);
        xGraphicStyles->insertByName("MyStyle1", Any(xStyle));
        uno::Reference<beans::XPropertySet> xPropertySet(xStyle, uno::UNO_QUERY_THROW);
        xPropertySet->setPropertyValue("FillColor", Any(COL_RED));
        xPropertySet->setPropertyValue("FillTransparence", Any(sal_Int16(40)));

        xStyle.set(xMSF->createInstance("com.sun.star.style.GraphicStyle"), uno::UNO_QUERY_THROW);
        xGraphicStyles->insertByName("MyStyle2", Any(xStyle));
        xStyle->setParentStyle("MyStyle1");

        xStyle.set(xMSF->createInstance("com.sun.star.style.GraphicStyle"), uno::UNO_QUERY_THROW);
        xGraphicStyles->insertByName("MyStyle3", Any(xStyle));
        xStyle->setParentStyle("MyStyle2");

        // create shape
        uno::Reference<drawing::XShape> xShape(
            xMSF->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY_THROW);

        uno::Reference<drawing::XDrawPagesSupplier> xDPS(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShapes> xShapes(xDPS->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY_THROW);
        xShapes->add(xShape);
        uno::Reference<beans::XPropertySet>(xShape, uno::UNO_QUERY_THROW)
            ->setPropertyValue("Style", Any(xStyle));
    }

    saveAndReload("calc8");

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDPS(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShapes> xShapes(xDPS->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xShape(xShapes->getByIndex(0), uno::UNO_QUERY_THROW);

        // check style hierarchy
        uno::Reference<style::XStyle> xStyle(xShape->getPropertyValue("Style"),
                                             uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(OUString("MyStyle3"), xStyle->getName());
        CPPUNIT_ASSERT_EQUAL(OUString("MyStyle2"), xStyle->getParentStyle());

        // check that styles have effect on shapes
        Color nColor;
        xShape->getPropertyValue("FillColor") >>= nColor;
        CPPUNIT_ASSERT_EQUAL(COL_RED, nColor);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(40),
                             xShape->getPropertyValue("FillTransparence").get<sal_Int16>());
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testCommentStyles)
{
    createScDoc("ods/comment.ods");

    {
        ScDocument* pDoc = getScDoc();

        ScAddress aPos(0, 0, 0);
        ScPostIt* pNote = pDoc->GetNote(aPos);
        CPPUNIT_ASSERT(pNote);

        auto pCaption = pNote->GetCaption();
        CPPUNIT_ASSERT(pCaption);

        // Check that we don't keep the shadow attribute as DF
        // (see ScNoteUtil::CreateNoteFromCaption)
        CPPUNIT_ASSERT_LESSEQUAL(SfxItemState::DEFAULT,
                                 pCaption->GetMergedItemSet().GetItemState(SDRATTR_SHADOW, false));

        auto pStyleSheet = &pDoc->GetStyleSheetPool()->Make("MyStyle1", SfxStyleFamily::Frame);
        auto& rSet = pStyleSheet->GetItemSet();
        rSet.Put(SvxFontHeightItem(1129, 100, EE_CHAR_FONTHEIGHT));

        pCaption->SetStyleSheet(static_cast<SfxStyleSheet*>(pStyleSheet), false);

        // Hidden comments use different code path on import
        pNote->ShowCaption(aPos, false);
    }

    saveAndReload("calc8");

    {
        ScDocument aDoc;
        aDoc.InitDrawLayer();
        aDoc.TransferTab(*getScDoc(), 0, 0);

        ScAddress aPos(0, 0, 0);
        ScPostIt* pNote = aDoc.GetNote(aPos);
        CPPUNIT_ASSERT(pNote);

        auto pCaption = pNote->GetOrCreateCaption(aPos);
        CPPUNIT_ASSERT(pCaption);

        // Check that the style was imported, and survived copying
        CPPUNIT_ASSERT_EQUAL(OUString("MyStyle1"), pCaption->GetStyleSheet()->GetName());
    }

    saveAndReload("Calc Office Open XML");

    {
        ScDocument* pDoc = getScDoc();

        ScAddress aPos(0, 0, 0);
        ScPostIt* pNote = pDoc->GetNote(aPos);
        CPPUNIT_ASSERT(pNote);

        auto pCaption = pNote->GetOrCreateCaption(aPos);
        CPPUNIT_ASSERT(pCaption);

        // Check that the style formatting is preserved
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(1129),
                             pCaption->GetMergedItemSet().Get(EE_CHAR_FONTHEIGHT).GetHeight());
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf119565)
{
    createScDoc("xlsx/tdf119565.xlsx");
    saveAndReload("Calc Office Open XML");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShapeProps(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 35
    // - Actual  : 0
    // i.e. line width inherited from theme lost after export.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(35),
                         xShapeProps->getPropertyValue("LineWidth").get<sal_Int32>());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 4
    // i.e. line joint inherited from theme lost after export.
    CPPUNIT_ASSERT_EQUAL(drawing::LineJoint_MITER,
                         xShapeProps->getPropertyValue("LineJoint").get<drawing::LineJoint>());
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf152980)
{
    createScDoc("csv/tdf152980.csv");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    saveAndReload("Calc Office Open XML");
    pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    // - Expected: The part between a and b does not change
    // - Actual  : Only the characters a and b remain
    CPPUNIT_ASSERT_EQUAL(OUString("a_x1_b"), pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("a_x01_b"), pDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("a_x001_b"), pDoc->GetString(0, 2, 0));

    // The character code does not change in both cases
    CPPUNIT_ASSERT_EQUAL(OUString("a_x0001_b"), pDoc->GetString(0, 3, 0));

    // The escape characters are handled correctly in both cases
    CPPUNIT_ASSERT_EQUAL(OUString("a_xfoo\nb"), pDoc->GetString(0, 4, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("a\tb"), pDoc->GetString(0, 5, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("a\nb"), pDoc->GetString(0, 6, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("a\n\nb"), pDoc->GetString(0, 7, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf100034)
{
    createScDoc("xlsx/tdf100034.xlsx");
    ScDocument* pDoc = getScDoc();

    // Clear print ranges (Format - Print Ranges - Clear)
    dispatchCommand(mxComponent, ".uno:DeletePrintArea", {});

    // Save and load back
    saveAndReload("Calc Office Open XML");

    // Check if the same print ranges are present
    pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), pDoc->GetPrintRangeCount(0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest4, testTdf157318)
{
    // This document has 2 named ranges; Test1 is global; Test2 is linked to Sheet1)
    createScDoc("ods/tdf157318.ods");
    ScDocument* pDoc = getScDoc();

    // Save as XLSX and load back
    saveAndReload("Calc Office Open XML");
    pDoc = getScDoc();

    // Check if there is one global named range
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1),
                         static_cast<sal_uInt16>(pDoc->GetRangeName()->size()));

    // Check if there is one named range in the first sheet
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1),
                         static_cast<sal_uInt16>(pDoc->GetRangeName(0)->size()));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
