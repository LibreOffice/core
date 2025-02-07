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
#include <global.hxx>
#include <dpcache.hxx>
#include <dpobject.hxx>
#include <clipparam.hxx>

#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/propertyvalue.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <formula/grammar.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>

#include <com/sun/star/sheet/XHeaderFooterContent.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScExportTest2 : public ScModelTestBase
{
public:
    ScExportTest2();
};

ScExportTest2::ScExportTest2()
    : ScModelTestBase(u"sc/qa/unit/data"_ustr)
{
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testGroupShape)
{
    createScDoc("xlsx/groupShape.xlsx");
    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pDoc = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:grpSp/xdr:grpSpPr");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testFreezePaneStartCellXLSX)
{
    // given a hand-mangled document with a newly-invalid topLeftCell for the active pane
    // where pane xSplit="5" ySplit="10" topLeftCell="A1" state="frozen"

    createScDoc("xlsx/freezePaneStartCell.xlsx");

    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    OUString sFirstNonFrozenCell = getXPath(pDoc, "//x:sheetViews//x:pane", "topLeftCell");

    // new versions of Excel complain if the start of the non-frozen section is inside the freeze
    CPPUNIT_ASSERT_EQUAL(u"F11"_ustr, sFirstNonFrozenCell);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testMatrixMultiplicationXLSX)
{
    createScDoc("xlsx/matrix-multiplication.xlsx");

    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    OUString CellFormulaRange = getXPath(pDoc, "/x:worksheet/x:sheetData/x:row[4]/x:c/x:f", "ref");

    // make sure that the CellFormulaRange is G5:G6.
    CPPUNIT_ASSERT_EQUAL(u"G5:G6"_ustr, CellFormulaRange);

    OUString CellFormulaType = getXPath(pDoc, "/x:worksheet/x:sheetData/x:row[4]/x:c/x:f", "t");

    // make sure that the CellFormulaType is array.
    CPPUNIT_ASSERT_EQUAL(u"array"_ustr, CellFormulaType);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testRefStringXLSX)
{
    createScDoc("xlsx/ref_string.xlsx");

    //make sure ref syntax gets saved for MSO-produced docs
    saveAndReload(u"Calc Office Open XML"_ustr);

    ScDocument* pDoc = getScDoc();
    ScCalcConfig aCalcConfig = pDoc->GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL(formula::FormulaGrammar::CONV_XL_A1, aCalcConfig.meStringRefAddressSyntax);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testRefStringConfigXLSX)
{
    // this doc is configured with CalcA1 ref syntax
    createScDoc("xlsx/empty.xlsx");

    saveAndReload(u"Calc Office Open XML"_ustr);

    ScDocument* pDoc = getScDoc();
    ScCalcConfig aConfig = pDoc->GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match",
                                 formula::FormulaGrammar::CONV_OOO,
                                 aConfig.meStringRefAddressSyntax);

    // this doc has no entry for ref syntax
    createScDoc("xlsx/empty-noconf.xlsx");

    pDoc = getScDoc();
    aConfig = pDoc->GetCalcConfig();
    // therefore after import, ref syntax should be set to CalcA1 | ExcelA1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match",
                                 formula::FormulaGrammar::CONV_A1_XL_A1,
                                 aConfig.meStringRefAddressSyntax);

    //set ref syntax to something else than ExcelA1 (native to xlsx format) ...
    aConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_XL_R1C1;
    pDoc->SetCalcConfig(aConfig);

    saveAndReload(u"Calc Office Open XML"_ustr);

    // ... and make sure it got saved
    pDoc = getScDoc();
    aConfig = pDoc->GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match",
                                 formula::FormulaGrammar::CONV_XL_R1C1,
                                 aConfig.meStringRefAddressSyntax);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testRefStringUnspecified)
{
    createScDoc();

    ScDocument* pDoc = getScDoc();
    ScCalcConfig aConfig = pDoc->GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default string ref syntax value doesn't match",
                                 formula::FormulaGrammar::CONV_UNSPECIFIED,
                                 aConfig.meStringRefAddressSyntax);

    // change formula syntax (i.e. not string ref syntax) to ExcelA1
    pDoc->SetGrammar(formula::FormulaGrammar::GRAM_NATIVE_XL_A1);

    saveAndReload(u"calc8"_ustr);

    // with string ref syntax at its default value, we should've saved ExcelA1
    pDoc = getScDoc();
    aConfig = pDoc->GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("String ref syntax doesn't match",
                                 formula::FormulaGrammar::CONV_XL_A1,
                                 aConfig.meStringRefAddressSyntax);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testHeaderImageODS)
{
    // Graphic as header background was lost on export.
    createScDoc("ods/header-image.ods");
    saveAndReload(u"calc8"_ustr);
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xPageStyles(
        xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName(u"Default"_ustr),
                                               uno::UNO_QUERY);

    uno::Reference<graphic::XGraphic> xGraphic;
    xStyle->getPropertyValue(u"HeaderBackGraphic"_ustr) >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testHeaderFooterContentODS)
{
    createScDoc("ods/header-footer-content.ods");
    saveAndReload(u"calc8"_ustr);
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xPageStyles(
        xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName(u"Default"_ustr),
                                               uno::UNO_QUERY);

    uno::Reference<css::sheet::XHeaderFooterContent> xContent;
    xStyle->getPropertyValue(u"RightPageHeaderContent"_ustr) >>= xContent;
    CPPUNIT_ASSERT(xContent.is());
    CPPUNIT_ASSERT_EQUAL(u"header"_ustr, xContent->getCenterText()->getString());

    xStyle->getPropertyValue(u"FirstPageHeaderContent"_ustr) >>= xContent;
    CPPUNIT_ASSERT(xContent.is());
    CPPUNIT_ASSERT_EQUAL(u"first page header"_ustr, xContent->getCenterText()->getString());

    xStyle->getPropertyValue(u"RightPageFooterContent"_ustr) >>= xContent;
    CPPUNIT_ASSERT(xContent.is());
    CPPUNIT_ASSERT_EQUAL(u"footer"_ustr, xContent->getCenterText()->getString());

    xStyle->getPropertyValue(u"FirstPageFooterContent"_ustr) >>= xContent;
    // First page footer content used to be lost upon export.
    CPPUNIT_ASSERT(xContent.is());
    CPPUNIT_ASSERT_EQUAL(u"first page footer"_ustr, xContent->getCenterText()->getString());
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTextDirectionXLSX)
{
    createScDoc("xlsx/writingMode.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment", "readingOrder",
                u"1"); //LTR
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[3]/x:alignment", "readingOrder",
                u"2"); //RTL
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf121260)
{
    createScDoc("ods/tdf121260.ods");

    ScDocument* pDoc = getScDoc();
    // change formula syntax (i.e. not string ref syntax) to ExcelA1
    FormulaGrammarSwitch aFGSwitch(pDoc, formula::FormulaGrammar::GRAM_NATIVE_XL_A1);

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pChart1 = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pChart1);

    // Without the fix in place, this test would have failed with
    // - Expected: Sheet1!$A$1:$A$2
    // - Actual  : sheet1 $A$1:$A$2
    assertXPathContent(pChart1,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:val/c:numRef/c:f",
                       u"Sheet1!$A$1:$A$2");
    assertXPathContent(pChart1,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[2]/c:val/c:numRef/c:f",
                       u"Sheet1!$B$1:$B$2");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf120168)
{
    createScDoc("xlsx/tdf120168.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: left
    // - Actual  : general
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment", "horizontal", u"left");
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[3]/x:alignment", "horizontal", u"right");

    // tdf#124741: without round-tripping customHeight, Excel wasn't applying defaultRowHeight
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    assertXPath(pSheet, "//x:sheetFormatPr", "customHeight", u"true");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf117266)
{
    createScDoc("xlsm/tdf117266_macroButton.xlsm");

    save(u"Calc MS Excel 2007 VBA XML"_ustr);
    xmlDocUniquePtr pVmlDrawing = parseExport(u"xl/drawings/vmlDrawing1.vml"_ustr);

    assertXPath(pVmlDrawing, "/xml/v:shape", "id", u"Button 1001");

    OUString sSpid = getXPath(pVmlDrawing, "/xml/v:shape", "spid");
    CPPUNIT_ASSERT(sSpid.startsWith("_x0000_s"));

    assertXPathContent(pVmlDrawing, "/xml/v:shape/v:textbox/div/font", u"Button 1 \"y\" z");
    // Why the xx:, I have no idea..., but it certainly doesn't work with just x:.
    assertXPathContent(pVmlDrawing, "/xml/v:shape//xx:FmlaMacro", u"Module1.Button1_Click");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf66668)
{
    // Would hang on exporting without the fix in place
    createScDoc("xlsx/tdf66668.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf130108)
{
    createScDoc("ods/tdf130108.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:b", "val", u"1");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:i", "val", u"0");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:color", "rgb", u"FFFFFFFF");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:sz", "val", u"10");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:fill/x:patternFill/x:bgColor", "rgb",
                u"FFCC0000");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf76949)
{
    createScDoc("ods/tdf76949.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c/x:f",
                       u"_xlfn.CHISQ.DIST(1,1,1)");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf107586)
{
    createScDoc("xlsx/tdf107586.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // Without the fix in place, this test would have failed with
    // XPath '/x:worksheet/x:sheetPr/x:tabColor' number of nodes is incorrect
    assertXPath(pSheet, "/x:worksheet/x:sheetPr/x:tabColor", "rgb", u"FF9BBB59");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf55417)
{
    createScDoc("xlsx/tdf55417.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[1]/x:alignment", 1);
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment", 1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf129985)
{
    createScDoc("xlsx/tdf129985.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode", u"m/d/yyyy");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf73063)
{
    createScDoc("xlsx/tdf73063.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode",
                u"[$-1C1A]dddd\", \"d\". \"mmmm\\ yyyy;@");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf95640_ods_to_xlsx)
{
    // Roundtripping sort options with user defined list to XLSX
    createScDoc("ods/tdf95640.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);

    assertXPath(pDoc, "//x:worksheet/x:autoFilter", "ref", u"A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "ref", u"A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "customList",
                u"Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf95640_ods_to_xlsx_with_standard_list)
{
    // Roundtripping sort options with user defined list to XLSX
    createScDoc("ods/tdf95640_standard_list.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);

    assertXPath(pDoc, "//x:worksheet/x:autoFilter", "ref", u"A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "ref", u"A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "customList",
                u"Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf95640_xlsx_to_xlsx)
{
    // XLSX Roundtripping sort options with custom sort list - note
    // that compared to ODS source documents above, here we _actually_
    // can use custom lists (beyond the global user defines), like
    // low, medium, high
    createScDoc("xlsx/tdf95640.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);

    assertXPath(pDoc, "//x:worksheet/x:autoFilter", "ref", u"A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "ref", u"A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "customList",
                u"Low,Medium,High");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testSortConditionRef)
{
    // Ascending sortCondition reference detected wrong without fix.
    // - Expected: B3:B2
    // - Actual  : A3:A2
    // - In <>, attribute 'ref' of '//x:worksheet/x:autoFilter/x:sortState/x:sortCondition' incorrect value.
    createScDoc("xlsx/sortconditionref.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);

    assertXPath(pDoc, "//x:worksheet/x:autoFilter", "ref", u"B2");
    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition", "ref", u"B3:B2");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testDateAutofilterXLSX)
{
    // XLSX Roundtripping autofilter with date list
    createScDoc("xlsx/dateAutofilter.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:autoFilter", "ref", u"A1:B4");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]", "day", u"02");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]", "month", u"03");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]", "year",
                u"2017");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]",
                "dateTimeGrouping", u"day");

    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]", "day", u"01");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]", "month", u"10");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]", "year",
                u"2014");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]",
                "dateTimeGrouping", u"day");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testDateAutofilterODS)
{
    createScDoc("ods/tdf142231.ods");

    save(u"calc8"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//table:filter/table:filter-and/table:filter-condition[1]", "value",
                u"Calc");
    assertXPath(pDoc, "//table:filter/table:filter-and/table:filter-condition[2]", "value",
                u"2021-05-04");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testAutofilterColorsODF)
{
    createScDoc("ods/autofilter-colors.ods");

    save(u"calc8"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[1]/table:filter/table:filter-and/"
                "table:filter-condition[1]",
                "value", u"#e8f2a1");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[1]/table:filter/table:filter-and/"
                "table:filter-condition[1][@loext:data-type='background-color']");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[1]/table:filter/table:filter-and/"
                "table:filter-condition[2]",
                "value", u"#3465a4");
    assertXPath(pDoc, "//table:database-ranges/table:database-range[1]/table:filter/"
                      "table:filter-and/table:filter-condition[2][@loext:data-type='text-color']");

    // tdf#142965 Check "none" value when automatic text color / no fill was selected
    assertXPath(pDoc, "//table:database-ranges/table:database-range[2]/table:filter/"
                      "table:filter-and/"
                      "table:filter-condition[1][@loext:data-type='background-color']");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[2]/table:filter/table:filter-and/"
                "table:filter-condition[1]",
                "value", u"transparent");
    assertXPath(pDoc, "//table:database-ranges/table:database-range[3]/table:filter/"
                      "table:filter-and/table:filter-condition[1][@loext:data-type='text-color']");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[3]/table:filter/table:filter-and/"
                "table:filter-condition[1]",
                "value", u"window-font-color");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testAutofilterColorsOOXML)
{
    {
        createScDoc("xlsx/autofilter-colors.xlsx");
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pTable1 = parseExport(u"xl/tables/table1.xml"_ustr);
        CPPUNIT_ASSERT(pTable1);
        sal_Int32 nDxfId
            = getXPath(pTable1, "/x:table/x:autoFilter/x:filterColumn/x:colorFilter", "dxfId")
                  .toInt32()
              + 1;

        xmlDocUniquePtr pStyles = parseExport(u"xl/styles.xml"_ustr);
        CPPUNIT_ASSERT(pStyles);
        OString sDxfXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfId)
                          + "]/x:fill/x:patternFill/x:fgColor");
        assertXPath(pStyles, sDxfXPath, "rgb", u"FFFFD7D7");
    }

    {
        createScDoc("xlsx/autofilter-colors-fg.xlsx");
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pTable1 = parseExport(u"xl/tables/table1.xml"_ustr);
        CPPUNIT_ASSERT(pTable1);
        sal_Int32 nDxfId
            = getXPath(pTable1, "/x:table/x:autoFilter/x:filterColumn/x:colorFilter", "dxfId")
                  .toInt32()
              + 1;

        xmlDocUniquePtr pStyles = parseExport(u"xl/styles.xml"_ustr);
        CPPUNIT_ASSERT(pStyles);
        OString sDxfXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfId)
                          + "]/x:fill/x:patternFill/x:fgColor");
        assertXPath(pStyles, sDxfXPath, "rgb", u"FF3465A4");
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testAutofilterTop10XLSX)
{
    createScDoc("xlsx/tdf143068_top10filter.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn", "colId", u"0");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:top10", "val", u"4");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testAutofilterColButton)
{
    // Without fix it will fail
    // - Expression: !xmlGetProp(pXmlNode, BAD_CAST(rAttribute.getStr()))
    // - In <>, XPath '//x:autoFilter/x:filterColumn' unexpected 'hiddenButton' attribute
    createScDoc("ods/autofilter-colbutton.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPathNoAttribute(pDoc, "//x:autoFilter/x:filterColumn", "hiddenButton");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf88657ODS)
{
    createScDoc("ods/tdf88657.ods");

    save(u"calc8"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//number:fraction", "min-denominator-digits", u"3");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf41722)
{
    createScDoc("xlsx/tdf41722.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[1]", "operator", u"containsText");
    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[2]", "operator", u"containsText");
    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[3]", "operator", u"containsText");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf113621)
{
    createScDoc("xlsx/tdf113621.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:conditionalFormatting", "sqref", u"A1:A1048576");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testEscapeCharInNumberFormatXLSX)
{
    createScDoc("xlsx/tdf81939.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode",
                u"00\\ 00\\ 00\\ 00\\ 00");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode",
                u"00\\.00\\.00\\.000\\.0"); // tdf#81939
    // "_-* #,##0\ _€_-;\-* #,##0\ _€_-;_-* "- "_€_-;_-@_-" // tdf#81222
    OUString rFormatStrExpected(u"_-* #,##0\\ _€_-;\\-* #,##0\\ _€_-;_-* \"- \"_€_-;_-@_-"_ustr);
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[4]", "formatCode", rFormatStrExpected);
    // "_-* #,##0" €"_-;\-* #,##0" €"_-;_-* "- €"_-;_-@_-");
    rFormatStrExpected = u"_-* #,##0\" €\"_-;\\-* #,##0\" €\"_-;_-* \"- €\"_-;_-@_-"_ustr;
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[5]", "formatCode", rFormatStrExpected);
    // remove escape char in fraction
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[6]", "formatCode",
                u"# ?/?;[RED]\\-# #/#####");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testNatNumInNumberFormatXLSX)
{
    createScDoc("ods/tdf79398_NatNum5.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode",
                u"[DBNum2][$-804]General;[RED][DBNum2][$-804]General");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testExponentWithoutSignFormatXLSX)
{
    createScDoc("ods/tdf102370_ExponentWithoutSign.ods");
    saveAndReload(u"Calc Office Open XML"_ustr);

    saveAndReload(u"calc8"_ustr);

    ScDocument* pDoc = getScDoc();
    sal_uInt32 nNumberFormat = pDoc->GetNumberFormat(0, 0, 0);
    const SvNumberformat* pNumberFormat = pDoc->GetFormatTable()->GetEntry(nNumberFormat);
    const OUString& rFormatStr = pNumberFormat->GetFormatstring();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format lost exponent without sign during Excel export",
                                 u"0.00E0"_ustr, rFormatStr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testExtendedLCIDXLSX)
{
    createScDoc("ods/tdf36038_ExtendedLCID.ods");

    saveAndReload(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDocXml = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pDocXml);
    // Check export
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[2]", "formatCode",
                u"[$-107041E]dd\\-mm\\-yyyy");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[3]", "formatCode",
                u"[$-D07041E]dd\\-mm\\-yyyy");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[4]", "formatCode",
                u"[$-1030411]dd\\-mm\\-ee");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[5]", "formatCode",
                u"[$-1B030411]dd\\-mm\\-ee");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[6]", "formatCode",
                u"[$-108040D]dd\\-mm\\-yyyy");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[7]", "formatCode",
                u"[$-108040D]dd\\-mm\\-yyyy");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[8]", "formatCode",
                u"[$-1060401]dd\\-mm\\-yyyy");

    // Check import
    ScDocument* pDoc = getScDoc();
    SvNumberFormatter* pNumFormatter = pDoc->GetFormatTable();
    const OUString aLang[5]
        = { u"[$-41E]"_ustr, u"[$-411]"_ustr, u"[$-40D]"_ustr, u"[$-401]"_ustr, u"[$-500]"_ustr };
    const OUString aCalendar[5]
        = { u"[~buddhist]DD-MM-YYYY"_ustr, u"DD-MM-EE"_ustr, u"[~jewish]DD-MM-YYYY"_ustr,
            u"[~hijri]DD-MM-YYYY"_ustr, u"[~dangi]YYYY/MM/DD"_ustr };
    // Note: ja-JP Gengou calendar is an implicit secondary (non-gregorian)
    // calendar, the explicit [~gengou] calendar modifier does not need to be
    // present, the E and EE keywords are used instead of YY and YYYY.
    for (sal_Int16 nCol = 1; nCol <= 2; nCol++)
    {
        for (sal_Int16 nRow = 1; nRow <= 4; nRow++)
        {
            sal_uInt32 nNumberFormat = pDoc->GetNumberFormat(nCol, nRow, 0);
            const SvNumberformat* pNumberFormat = pNumFormatter->GetEntry(nNumberFormat);
            const OUString& rFormatStr = pNumberFormat->GetFormatstring();
            const OUString aExpectedFormatStr
                = aLang[nRow - 1] + ((nCol == 2 && nRow != 3) ? u"[NatNum1]"_ustr : OUString())
                  + aCalendar[nRow - 1];

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format lost extended LCID during Excel export",
                                         aExpectedFormatStr, rFormatStr);
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testHiddenRepeatedRowsODS)
{
    createScDoc();

    {
        ScDocument* pDoc = getScDoc();
        pDoc->SetRowHidden(0, 20, 0, true);
    }

    saveAndReload(u"calc8"_ustr);
    ScDocument* pDoc = getScDoc();
    SCROW nFirstRow = 0;
    SCROW nLastRow = 0;
    bool bHidden = pDoc->RowHidden(0, 0, &nFirstRow, &nLastRow);
    CPPUNIT_ASSERT(bHidden);
    CPPUNIT_ASSERT_EQUAL(SCROW(0), nFirstRow);
    CPPUNIT_ASSERT_EQUAL(SCROW(20), nLastRow);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testHyperlinkTargetFrameODS)
{
    createScDoc("ods/hyperlink_frame.ods");

    ScDocument* pDoc = getScDoc();
    const EditTextObject* pEditText = pDoc->GetEditText(ScAddress(2, 5, 0));
    CPPUNIT_ASSERT(pEditText);

    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the URL data.", pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to get the URL data.", text::textfield::Type::URL,
                                 pData->GetClassId());

    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    OUString aTargetFrame = pURLData->GetTargetFrame();
    CPPUNIT_ASSERT_EQUAL(u"_blank"_ustr, aTargetFrame);

    save(u"calc8"_ustr);
    xmlDocUniquePtr pDocXml = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pDocXml);
    OUString aTargetFrameExport
        = getXPath(pDocXml,
                   "/office:document-content/office:body/office:spreadsheet/table:table/"
                   "table:table-row[2]/table:table-cell[2]/text:p/text:a",
                   "target-frame-name");
    CPPUNIT_ASSERT_EQUAL(u"_blank"_ustr, aTargetFrameExport);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testOpenDocumentAsReadOnly)
{
    uno::Sequence<beans::PropertyValue> aParams
        = { comphelper::makePropertyValue(u"Silent"_ustr, true) };

    loadWithParams(createFileURL(u"xlsx/open-as-read-only.xlsx"), aParams);
    ScDocShell* pDocSh = getScDocShell();
    CPPUNIT_ASSERT(pDocSh->IsSecurityOptOpenReadOnly());

    saveWithParams(uno::Sequence<beans::PropertyValue>());
    loadWithParams(maTempFile.GetURL(), aParams);

    pDocSh = getScDocShell();
    CPPUNIT_ASSERT(pDocSh->IsSecurityOptOpenReadOnly());
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testKeepSettingsOfBlankRows)
{
    createScDoc("xlsx/tdf41425.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // saved blank row with not default setting in A2
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row", 2);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf133595)
{
    createScDoc("xlsx/tdf133595.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // without the fix in place, mc:AlternateContent would have been added to sheet1
    assertXPath(pSheet, "/x:worksheet/mc:AlternateContent", 0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf134769)
{
    createScDoc("xlsx/tdf134769.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // without the fix in place, the legacyDrawing would have been exported after the checkbox
    // and Excel would have claimed the document is corrupted
    // Use their ids to check the order
    assertXPath(pSheet, "/x:worksheet/x:drawing", "id", u"rId2");
    assertXPath(pSheet, "/x:worksheet/x:legacyDrawing", "id", u"rId3");
    assertXPath(pSheet,
                "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/"
                "mc:Choice/x:control",
                "id", u"rId4");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf106181)
{
    createScDoc("ods/tdf106181.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet,
                "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/"
                "mc:Choice/x:control",
                "name", u"Check Box");
    assertXPath(pSheet,
                "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/"
                "mc:Choice/x:control/x:controlPr",
                "altText", u"Check Box 1");

    xmlDocUniquePtr pDrawing = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(
        pDrawing,
        "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr",
        "name", u"Check Box 1");
    assertXPath(
        pDrawing,
        "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr",
        "descr", u"Check Box");
    assertXPath(
        pDrawing,
        "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr",
        "hidden", u"0");

    xmlDocUniquePtr pVmlDrawing = parseExport(u"xl/drawings/vmlDrawing1.vml"_ustr);
    CPPUNIT_ASSERT(pVmlDrawing);
    assertXPathContent(pVmlDrawing, "//xx:ClientData/xx:FmlaLink", u"$D$9");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf145057)
{
    createScDoc("xlsx/tdf145057.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/tables/table1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:colorFilter", "dxfId", u"1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf105272)
{
    createScDoc("xlsx/tdf105272.xlsx");
    saveAndReload(u"Calc Office Open XML"_ustr);
    ScDocument* pDoc = getScDoc();
    //without the fix in place,it would fail
    //Expected: Table1[[#This Row],[Total]]/Table1[[#This Row],['# Athletes]]
    //Actual  : table1[[#this row],[total]]/table1[[#this row],['# athletes]]

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong formula", u"=Table1[[#This Row],[Total]]/Table1[[#This Row],['# Athletes]]"_ustr,
        pDoc->GetFormula(7, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf118990)
{
    createScDoc("xlsx/tdf118990.xlsx");
    saveAndReload(u"Calc Office Open XML"_ustr);
    ScDocument* pDoc = getScDoc();

    // TODO: also test A1, which contains a UNC reference to \\localhost\share\lookupsource.xlsx,
    // but currently looses "localhost" part when normalized in INetURLObject, becoming
    // file:///share/lookupsource.xlsx - which is incorrect, since it points to local filesystem
    // and not to Windows network share.

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong Windows share (using host IP) URL in A2",
        u"=VLOOKUP(B1,'file://192.168.1.1/share/lookupsource.xlsx'#$Sheet1.A1:B5,2)"_ustr,
        pDoc->GetFormula(0, 1, 0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong Windows share (using hostname) URL in A3",
        u"=VLOOKUP(B1,'file://NETWORKHOST/share/lookupsource.xlsx'#$Sheet1.A1:B5,2)"_ustr,
        pDoc->GetFormula(0, 2, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf121612)
{
    createScDoc("ods/tdf121612.ods");
    saveAndReload(u"Calc Office Open XML"_ustr);

    ScDocument* pDoc = getScDoc();

    // There should be a pivot table
    CPPUNIT_ASSERT(pDoc->HasPivotTable());

    // DP collection is not lost after export and has one entry
    ScDPCollection* pDPColl = pDoc->GetDPCollection();
    CPPUNIT_ASSERT(pDPColl);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDPColl->GetCount());
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf112936)
{
    createScDoc("xlsx/tdf112936.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/pivotCache/pivotCacheDefinition1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:pivotCacheDefinition", "recordCount", u"4");
    assertXPath(pDoc, "//x:pivotCacheDefinition", "createdVersion", u"3");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testXltxExport)
{
    // Create new document
    createScDoc();

    // Export as template and check content type
    save(u"Calc MS Excel 2007 XML Template"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"[Content_Types].xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/ContentType:Types/ContentType:Override[@PartName='/xl/workbook.xml']",
                "ContentType",
                u"application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testPivotCacheAfterExportXLSX)
{
    createScDoc("ods/numgroup_example.ods");

    // export only
    save(u"Calc Office Open XML"_ustr);

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc->HasPivotTable());

    // Two pivot tables
    ScDPCollection* pDPColl = pDoc->GetDPCollection();
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
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf114969XLSX)
{
    createScDoc("ods/sheet_name_with_dots.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[1]", "location", u"'1.1.1.1'!C1");
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[2]", "location", u"'1.1.1.1'!C2");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf115192XLSX)
{
    createScDoc("xlsx/test_115192.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/drawings/_rels/drawing1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']", "TargetMode",
                u"External");
    assertXPathNoAttribute(pDoc, "/rels:Relationships/rels:Relationship[@Id='rId2']", "TargetMode");
    assertXPath(pDoc, "/rels:Relationships/rels:Relationship[@Id='rId3']", "TargetMode",
                u"External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf142764)
{
    createScDoc("ods/tdf142764.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:headerFooter", "differentOddEven", u"true");
    assertXPath(pSheet, "/x:worksheet/x:headerFooter", "differentFirst", u"true");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf91634XLSX)
{
    createScDoc("xlsx/image_hyperlink.xlsx");
    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pDoc = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:pic/xdr:nvPicPr/xdr:cNvPr/a:hlinkClick", 1);

    xmlDocUniquePtr pXmlRels = parseExport(u"xl/drawings/_rels/drawing1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                u"https://www.google.com/");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "TargetMode",
                u"External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testValidationCopyPaste)
{
    createScDoc("ods/validation-copypaste.ods");
    ScDocument* pDoc = getScDoc();

    // Copy B1 from src doc to clip
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScRange aSrcRange(1, 0, 1);
    ScClipParam aClipParam(aSrcRange, false);
    ScMarkData aMark(pDoc->GetSheetLimits());
    aMark.SetMarkArea(aSrcRange);
    pDoc->CopyToClip(aClipParam, &aClipDoc, &aMark, false, false);

    // Create second document, paste B1 from clip
    createScDoc();
    pDoc = getScDoc();
    ScRange aDstRange(1, 0, 0);
    ScMarkData aMark2(pDoc->GetSheetLimits());
    aMark2.SetMarkArea(aDstRange);
    pDoc->CopyFromClip(aDstRange, aMark2, InsertDeleteFlags::ALL, nullptr, &aClipDoc);

    // save as XLSX
    save(u"Calc Office Open XML"_ustr);

    // check validation
    xmlDocUniquePtr pDocXml = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDocXml);
    assertXPathContent(pDocXml, "/x:worksheet/x:dataValidations/x:dataValidation/x:formula1",
                       u"#REF!");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf115159)
{
    createScDoc("xlsx/tdf115159.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/workbook.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName", 1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf112567)
{
    // Set the system locale to Hungarian (a language with different range separator)
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString(u"hu-HU"_ustr);
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    createScDoc("xlsx/tdf112567.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/workbook.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName", 1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf75702)
{
    // The problem was that line breaks were not imported.
    static constexpr OUString sA1(u"line1\nline2"_ustr);

    createScDoc("ods/tdf75702_textLineBreak.ods");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("load a1", sA1, pDoc->GetString(0, 0, 0));

    saveAndReload(u"calc8"_ustr);
    pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("reload a1", sA1, pDoc->GetString(0, 0, 0));

    save(u"calc8"_ustr);
    xmlDocUniquePtr pContent = parseExport(u"content.xml"_ustr);
    assertXPath(pContent, "//table:table-row[1]/table:table-cell/text:p", 2);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf103829)
{
    // The problem was that tabspaces were not imported or exported at all.
    // These strings match the current implementations of CELLTYPE_EDIT and CELLTYPE_STRING.
    static constexpr OUString sA1(u"\x001Leading tab\nTHREE tabs inside: [\x001\x001\x001]"_ustr);
    static constexpr OUString sA2(u"\tLeading tab. THREE tabs inside: [\t\t\t]"_ustr);

    createScDoc("ods/tdf103829_textTab.ods");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("load a1", sA1, pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("load a2", sA2, pDoc->GetString(0, 1, 0));

    saveAndReload(u"calc8"_ustr);
    pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("reload a1", sA1, pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("reload a2", sA2, pDoc->GetString(0, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf122191)
{
    // Set the system locale to Hungarian
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString(u"hu-HU"_ustr);
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    createScDoc("xlsx/tdf122191.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(u"IGAZ"_ustr, pDoc->GetString(0, 0, 0));

    saveAndReload(u"Calc Office Open XML"_ustr);

    pDoc = getScDoc();
    // Without the fix in place, this test would have failed with
    // - Expected: IGAZ
    // - Actual  : BOOL00AN
    CPPUNIT_ASSERT_EQUAL(u"IGAZ"_ustr, pDoc->GetString(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf142881)
{
    createScDoc("xlsx/tdf142881.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDrawing1 = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDrawing1);

    // Verify that the shapes are rotated and positioned in the expected way
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:from/xdr:col", u"11");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:from/xdr:row", u"0");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:to/xdr:col", u"12");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:to/xdr:row", u"19");

    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:from/xdr:col", u"2");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:from/xdr:row", u"8");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:to/xdr:col", u"7");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:to/xdr:row", u"10");

    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:from/xdr:col", u"10");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:from/xdr:row", u"9");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:to/xdr:col", u"11");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:to/xdr:row", u"26");

    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:from/xdr:col", u"2");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:from/xdr:row", u"17");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:to/xdr:col", u"8");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:to/xdr:row", u"19");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf112567b)
{
    // Set the system locale to Hungarian (a language with different range separator)
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString(u"hu-HU"_ustr);
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    createScDoc("ods/tdf112567.ods");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/workbook.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName", 1);

    //and it contains "," instead of ";"
    assertXPathContent(pDoc, "/x:workbook/x:definedNames/x:definedName[1]",
                       u"Sheet1!$A:$A,Sheet1!$1:$1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf123645XLSX)
{
    createScDoc("xlsx/chart_hyperlink.xlsx");
    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pDoc = parseExport(u"xl/drawings/drawing1.xml"_ustr);
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

    xmlDocUniquePtr pXmlRels = parseExport(u"xl/drawings/_rels/drawing1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "TargetMode",
                u"External");
    assertXPathNoAttribute(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId3']",
                           "TargetMode");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId5']", "TargetMode",
                u"External");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                u"file:///C:/TEMP/test.xlsx");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId3']", "Target",
                u"#Sheet2!A1");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId5']", "Target",
                u"https://bugs.documentfoundation.org/show_bug.cgi?id=123645");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf125173XLSX)
{
    createScDoc("ods/text_box_hyperlink.ods");
    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pDoc = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr/a:hlinkClick", 1);

    xmlDocUniquePtr pXmlRels = parseExport(u"xl/drawings/_rels/drawing1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target",
                u"http://www.google.com/");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']", "TargetMode",
                u"External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf79972XLSX)
{
    createScDoc("xlsx/tdf79972.xlsx");
    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink", "ref", u"A1");

    xmlDocUniquePtr pXmlRels = parseExport(u"xl/worksheets/_rels/sheet1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "Target",
                u"https://bugs.documentfoundation.org/show_bug.cgi?id=79972");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "TargetMode", u"External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf126024XLSX)
{
    createScDoc("xlsx/hyperlink_formula.xlsx");
    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink", "ref", u"A2");

    xmlDocUniquePtr pXmlRels = parseExport(u"xl/worksheets/_rels/sheet1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "Target",
                u"https://bugs.documentfoundation.org/");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship", "TargetMode", u"External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testEnhancedProtectionRangeShorthand)
{
    createScDoc("xlsx/enhancedProtectionRangeShorthand.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    // the shorthand version was not recognized as a valid range on import
    assertXPath(pDoc, "//x:protectedRanges/x:protectedRange[1]", "sqref", u"C:C");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf91332)
{
    createScDoc("xlsx/tdf91332.xlsx");
    saveAndReload(u"Calc Office Open XML"_ustr);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShapeProps(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: rgba[90cf47ff]
    // - Actual  : rgba[ffffffff]
    // i.e. fill color inherited from theme lost after export.
    Color nColor;
    xShapeProps->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x90cf47), nColor);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf137543XLSX)
{
    // LET function test
    createScDoc("xlsx/tdf137543.xlsx");

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c/x:f",
                       u"_xlfn.LET(_xlpm.first,15,_xlpm.second,10,SUM(_xlpm.first,_xlpm.second))");

    assertXPathContent(
        pSheet, "/x:worksheet/x:sheetData/x:row[3]/x:c[5]/x:f",
        u"_xlfn.LET(_xlpm.first,B5:E15,_xlfn.CHOOSEROWS(_xlpm.first, 1, 3, 5, 7, 9, 11))");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
