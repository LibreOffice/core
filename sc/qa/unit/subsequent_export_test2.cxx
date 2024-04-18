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
    : ScModelTestBase("sc/qa/unit/data")
{
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testGroupShape)
{
    createScDoc("xlsx/groupShape.xlsx");
    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:grpSp/xdr:grpSpPr"_ostr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testMatrixMultiplicationXLSX)
{
    createScDoc("xlsx/matrix-multiplication.xlsx");

    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    OUString CellFormulaRange
        = getXPath(pDoc, "/x:worksheet/x:sheetData/x:row[4]/x:c/x:f"_ostr, "ref"_ostr);

    // make sure that the CellFormulaRange is G5:G6.
    CPPUNIT_ASSERT_EQUAL(OUString("G5:G6"), CellFormulaRange);

    OUString CellFormulaType
        = getXPath(pDoc, "/x:worksheet/x:sheetData/x:row[4]/x:c/x:f"_ostr, "t"_ostr);

    // make sure that the CellFormulaType is array.
    CPPUNIT_ASSERT_EQUAL(OUString("array"), CellFormulaType);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testRefStringXLSX)
{
    createScDoc("xlsx/ref_string.xlsx");

    //make sure ref syntax gets saved for MSO-produced docs
    saveAndReload("Calc Office Open XML");

    ScDocument* pDoc = getScDoc();
    ScCalcConfig aCalcConfig = pDoc->GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL(formula::FormulaGrammar::CONV_XL_A1, aCalcConfig.meStringRefAddressSyntax);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testRefStringConfigXLSX)
{
    // this doc is configured with CalcA1 ref syntax
    createScDoc("xlsx/empty.xlsx");

    saveAndReload("Calc Office Open XML");

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

    saveAndReload("Calc Office Open XML");

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

    saveAndReload("calc8");

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
    saveAndReload("calc8");
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xPageStyles(xStyleFamilies->getByName("PageStyles"),
                                                       uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName("Default"), uno::UNO_QUERY);

    uno::Reference<graphic::XGraphic> xGraphic;
    xStyle->getPropertyValue("HeaderBackGraphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testHeaderFooterContentODS)
{
    createScDoc("ods/header-footer-content.ods");
    saveAndReload("calc8");
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
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
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTextDirectionXLSX)
{
    createScDoc("xlsx/writingMode.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment"_ostr, "readingOrder"_ostr,
                "1"); //LTR
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[3]/x:alignment"_ostr, "readingOrder"_ostr,
                "2"); //RTL
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf121260)
{
    createScDoc("ods/tdf121260.ods");

    ScDocument* pDoc = getScDoc();
    // change formula syntax (i.e. not string ref syntax) to ExcelA1
    FormulaGrammarSwitch aFGSwitch(pDoc, formula::FormulaGrammar::GRAM_NATIVE_XL_A1);

    save("Calc Office Open XML");
    xmlDocUniquePtr pChart1 = parseExport("xl/charts/chart1.xml");
    CPPUNIT_ASSERT(pChart1);

    // Without the fix in place, this test would have failed with
    // - Expected: Sheet1!$A$1:$A$2
    // - Actual  : sheet1 $A$1:$A$2
    assertXPathContent(
        pChart1, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:val/c:numRef/c:f"_ostr,
        "Sheet1!$A$1:$A$2");
    assertXPathContent(
        pChart1, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[2]/c:val/c:numRef/c:f"_ostr,
        "Sheet1!$B$1:$B$2");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf120168)
{
    createScDoc("xlsx/tdf120168.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: left
    // - Actual  : general
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment"_ostr, "horizontal"_ostr,
                "left");
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[3]/x:alignment"_ostr, "horizontal"_ostr,
                "right");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf117266)
{
    createScDoc("xlsm/tdf117266_macroButton.xlsm");

    save("Calc MS Excel 2007 VBA XML");
    xmlDocUniquePtr pVmlDrawing = parseExport("xl/drawings/vmlDrawing1.vml");

    OUString sName = getXPath(pVmlDrawing, "/xml/v:shape"_ostr, "id"_ostr);
    CPPUNIT_ASSERT_EQUAL(OUString("Button 1001"), sName);

    OUString sSpid = getXPath(pVmlDrawing, "/xml/v:shape"_ostr, "spid"_ostr);
    CPPUNIT_ASSERT(sSpid.startsWith("_x0000_s"));

    assertXPathContent(pVmlDrawing, "/xml/v:shape/v:textbox/div/font"_ostr, "Button 1 \"y\" z");
    // Why the xx:, I have no idea..., but it certainly doesn't work with just x:.
    assertXPathContent(pVmlDrawing, "/xml/v:shape//xx:FmlaMacro"_ostr, "Module1.Button1_Click");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf66668)
{
    // Would hang on exporting without the fix in place
    createScDoc("xlsx/tdf66668.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pDoc);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf130108)
{
    createScDoc("ods/tdf130108.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:b"_ostr, "val"_ostr, "1");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:i"_ostr, "val"_ostr, "0");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:color"_ostr, "rgb"_ostr, "FFFFFFFF");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:sz"_ostr, "val"_ostr, "10");
    assertXPath(pDoc, "/x:styleSheet/x:dxfs/x:dxf/x:fill/x:patternFill/x:bgColor"_ostr, "rgb"_ostr,
                "FFCC0000");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf76949)
{
    createScDoc("ods/tdf76949.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row/x:c/x:f"_ostr,
                       "_xlfn.CHISQ.DIST(1,1,1)");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf107586)
{
    createScDoc("xlsx/tdf107586.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // Without the fix in place, this test would have failed with
    // XPath '/x:worksheet/x:sheetPr/x:tabColor' number of nodes is incorrect
    assertXPath(pSheet, "/x:worksheet/x:sheetPr/x:tabColor"_ostr, "rgb"_ostr, "FF9BBB59");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf55417)
{
    createScDoc("xlsx/tdf55417.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[1]/x:alignment"_ostr, 1);
    assertXPath(pDoc, "/x:styleSheet/x:cellXfs/x:xf[2]/x:alignment"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf129985)
{
    createScDoc("xlsx/tdf129985.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]"_ostr, "formatCode"_ostr, "m/d/yyyy");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf73063)
{
    createScDoc("xlsx/tdf73063.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]"_ostr, "formatCode"_ostr,
                "[$-1C1A]dddd\", \"d\". \"mmmm\\ yyyy;@");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf95640_ods_to_xlsx)
{
    // Roundtripping sort options with user defined list to XLSX
    createScDoc("ods/tdf95640.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter"_ostr, "ref"_ostr, "A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition"_ostr, "ref"_ostr,
                "A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition"_ostr,
                "customList"_ostr, "Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf95640_ods_to_xlsx_with_standard_list)
{
    // Roundtripping sort options with user defined list to XLSX
    createScDoc("ods/tdf95640_standard_list.ods");
    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter"_ostr, "ref"_ostr, "A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition"_ostr, "ref"_ostr,
                "A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition"_ostr,
                "customList"_ostr, "Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf95640_xlsx_to_xlsx)
{
    // XLSX Roundtripping sort options with custom sort list - note
    // that compared to ODS source documents above, here we _actually_
    // can use custom lists (beyond the global user defines), like
    // low, medium, high
    createScDoc("xlsx/tdf95640.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter"_ostr, "ref"_ostr, "A1:B4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition"_ostr, "ref"_ostr,
                "A2:A4");

    assertXPath(pDoc, "//x:worksheet/x:autoFilter/x:sortState/x:sortCondition"_ostr,
                "customList"_ostr, "Low,Medium,High");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testDateAutofilterXLSX)
{
    // XLSX Roundtripping autofilter with date list
    createScDoc("xlsx/dateAutofilter.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:autoFilter"_ostr, "ref"_ostr, "A1:B4");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]"_ostr, "day"_ostr,
                "02");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]"_ostr,
                "month"_ostr, "03");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]"_ostr,
                "year"_ostr, "2017");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]"_ostr,
                "dateTimeGrouping"_ostr, "day");

    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]"_ostr, "day"_ostr,
                "01");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]"_ostr,
                "month"_ostr, "10");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]"_ostr,
                "year"_ostr, "2014");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[2]"_ostr,
                "dateTimeGrouping"_ostr, "day");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testDateAutofilterODS)
{
    createScDoc("ods/tdf142231.ods");

    save("calc8");
    xmlDocUniquePtr pDoc = parseExport("content.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//table:filter/table:filter-and/table:filter-condition[1]"_ostr,
                "value"_ostr, "Calc");
    assertXPath(pDoc, "//table:filter/table:filter-and/table:filter-condition[2]"_ostr,
                "value"_ostr, "2021-05-04");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testAutofilterColorsODF)
{
    createScDoc("ods/autofilter-colors.ods");

    save("calc8");
    xmlDocUniquePtr pDoc = parseExport("content.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[1]/table:filter/table:filter-and/"
                "table:filter-condition[1]"_ostr,
                "value"_ostr, "#e8f2a1");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[1]/table:filter/table:filter-and/"
                "table:filter-condition[1][@loext:data-type='background-color']"_ostr);
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[1]/table:filter/table:filter-and/"
                "table:filter-condition[2]"_ostr,
                "value"_ostr, "#3465a4");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[1]/table:filter/"
                "table:filter-and/table:filter-condition[2][@loext:data-type='text-color']"_ostr);

    // tdf#142965 Check "none" value when automatic text color / no fill was selected
    assertXPath(pDoc, "//table:database-ranges/table:database-range[2]/table:filter/"
                      "table:filter-and/"
                      "table:filter-condition[1][@loext:data-type='background-color']"_ostr);
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[2]/table:filter/table:filter-and/"
                "table:filter-condition[1]"_ostr,
                "value"_ostr, "transparent");
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[3]/table:filter/"
                "table:filter-and/table:filter-condition[1][@loext:data-type='text-color']"_ostr);
    assertXPath(pDoc,
                "//table:database-ranges/table:database-range[3]/table:filter/table:filter-and/"
                "table:filter-condition[1]"_ostr,
                "value"_ostr, "window-font-color");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testAutofilterColorsOOXML)
{
    {
        createScDoc("xlsx/autofilter-colors.xlsx");
        save("Calc Office Open XML");
        xmlDocUniquePtr pTable1 = parseExport("xl/tables/table1.xml");
        CPPUNIT_ASSERT(pTable1);
        sal_Int32 nDxfId
            = getXPath(pTable1, "/x:table/x:autoFilter/x:filterColumn/x:colorFilter"_ostr,
                       "dxfId"_ostr)
                  .toInt32()
              + 1;

        xmlDocUniquePtr pStyles = parseExport("xl/styles.xml");
        CPPUNIT_ASSERT(pStyles);
        OString sDxfXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfId)
                          + "]/x:fill/x:patternFill/x:fgColor");
        assertXPath(pStyles, sDxfXPath, "rgb"_ostr, "FFFFD7D7");
    }

    {
        createScDoc("xlsx/autofilter-colors-fg.xlsx");
        save("Calc Office Open XML");
        xmlDocUniquePtr pTable1 = parseExport("xl/tables/table1.xml");
        CPPUNIT_ASSERT(pTable1);
        sal_Int32 nDxfId
            = getXPath(pTable1, "/x:table/x:autoFilter/x:filterColumn/x:colorFilter"_ostr,
                       "dxfId"_ostr)
                  .toInt32()
              + 1;

        xmlDocUniquePtr pStyles = parseExport("xl/styles.xml");
        CPPUNIT_ASSERT(pStyles);
        OString sDxfXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfId)
                          + "]/x:fill/x:patternFill/x:fgColor");
        assertXPath(pStyles, sDxfXPath, "rgb"_ostr, "FF3465A4");
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testAutofilterTop10XLSX)
{
    createScDoc("xlsx/tdf143068_top10filter.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn"_ostr, "colId"_ostr, "0");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:top10"_ostr, "val"_ostr, "4");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf88657ODS)
{
    createScDoc("ods/tdf88657.ods");

    save("calc8");
    xmlDocUniquePtr pDoc = parseExport("styles.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//number:fraction"_ostr, "min-denominator-digits"_ostr, "3");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf41722)
{
    createScDoc("xlsx/tdf41722.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[1]"_ostr, "operator"_ostr,
                "containsText");
    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[2]"_ostr, "operator"_ostr,
                "containsText");
    assertXPath(pDoc, "//x:conditionalFormatting/x:cfRule[3]"_ostr, "operator"_ostr,
                "containsText");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf113621)
{
    createScDoc("xlsx/tdf113621.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:conditionalFormatting"_ostr, "sqref"_ostr, "A1:A1048576");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testEscapeCharInNumberFormatXLSX)
{
    createScDoc("xlsx/tdf81939.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[2]"_ostr, "formatCode"_ostr,
                "00\\ 00\\ 00\\ 00\\ 00");
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]"_ostr, "formatCode"_ostr,
                "00\\.00\\.00\\.000\\.0"); // tdf#81939
    // "_-* #,##0\ _€_-;\-* #,##0\ _€_-;_-* "- "_€_-;_-@_-" // tdf#81222
    OUString rFormatStrExpected(u"_-* #,##0\\ _€_-;\\-* #,##0\\ _€_-;_-* \"- \"_€_-;_-@_-"_ustr);
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[4]"_ostr, "formatCode"_ostr,
                rFormatStrExpected);
    // "_-* #,##0" €"_-;\-* #,##0" €"_-;_-* "- €"_-;_-@_-");
    rFormatStrExpected = u"_-* #,##0\" €\"_-;\\-* #,##0\" €\"_-;_-* \"- €\"_-;_-@_-"_ustr;
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[5]"_ostr, "formatCode"_ostr,
                rFormatStrExpected);
    // remove escape char in fraction
    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[6]"_ostr, "formatCode"_ostr,
                "# ?/?;[RED]\\-# #/#####");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testNatNumInNumberFormatXLSX)
{
    createScDoc("ods/tdf79398_NatNum5.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/x:styleSheet/x:numFmts/x:numFmt[3]"_ostr, "formatCode"_ostr,
                "[DBNum2][$-804]General;[RED][DBNum2][$-804]General");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testExponentWithoutSignFormatXLSX)
{
    createScDoc("ods/tdf102370_ExponentWithoutSign.ods");
    saveAndReload("Calc Office Open XML");

    saveAndReload("calc8");

    ScDocument* pDoc = getScDoc();
    sal_uInt32 nNumberFormat = pDoc->GetNumberFormat(0, 0, 0);
    const SvNumberformat* pNumberFormat = pDoc->GetFormatTable()->GetEntry(nNumberFormat);
    const OUString& rFormatStr = pNumberFormat->GetFormatstring();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format lost exponent without sign during Excel export",
                                 OUString("0.00E0"), rFormatStr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testExtendedLCIDXLSX)
{
    createScDoc("ods/tdf36038_ExtendedLCID.ods");

    saveAndReload("Calc Office Open XML");
    xmlDocUniquePtr pDocXml = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pDocXml);
    // Check export
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[2]"_ostr, "formatCode"_ostr,
                "[$-107041E]dd\\-mm\\-yyyy");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[3]"_ostr, "formatCode"_ostr,
                "[$-D07041E]dd\\-mm\\-yyyy");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[4]"_ostr, "formatCode"_ostr,
                "[$-1030411]dd\\-mm\\-ee");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[5]"_ostr, "formatCode"_ostr,
                "[$-1B030411]dd\\-mm\\-ee");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[6]"_ostr, "formatCode"_ostr,
                "[$-108040D]dd\\-mm\\-yyyy");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[7]"_ostr, "formatCode"_ostr,
                "[$-108040D]dd\\-mm\\-yyyy");
    assertXPath(pDocXml, "/x:styleSheet/x:numFmts/x:numFmt[8]"_ostr, "formatCode"_ostr,
                "[$-1060401]dd\\-mm\\-yyyy");

    // Check import
    ScDocument* pDoc = getScDoc();
    SvNumberFormatter* pNumFormatter = pDoc->GetFormatTable();
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
            sal_uInt32 nNumberFormat = pDoc->GetNumberFormat(nCol, nRow, 0);
            const SvNumberformat* pNumberFormat = pNumFormatter->GetEntry(nNumberFormat);
            const OUString& rFormatStr = pNumberFormat->GetFormatstring();
            const OUString aExpectedFormatStr
                = aLang[nRow - 1] + ((nCol == 2 && nRow != 3) ? OUString("[NatNum1]") : OUString())
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

    saveAndReload("calc8");
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
    CPPUNIT_ASSERT_EQUAL(OUString("_blank"), aTargetFrame);

    save("calc8");
    xmlDocUniquePtr pDocXml = parseExport("content.xml");
    CPPUNIT_ASSERT(pDocXml);
    OUString aTargetFrameExport
        = getXPath(pDocXml,
                   "/office:document-content/office:body/office:spreadsheet/table:table/"
                   "table:table-row[2]/table:table-cell[2]/text:p/text:a"_ostr,
                   "target-frame-name"_ostr);
    CPPUNIT_ASSERT_EQUAL(OUString("_blank"), aTargetFrameExport);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testOpenDocumentAsReadOnly)
{
    uno::Sequence<beans::PropertyValue> aParams = { comphelper::makePropertyValue("Silent", true) };

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

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // saved blank row with not default setting in A2
    assertXPath(pSheet, "/x:worksheet/x:sheetData/x:row"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf133595)
{
    createScDoc("xlsx/tdf133595.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // without the fix in place, mc:AlternateContent would have been added to sheet1
    assertXPath(pSheet, "/x:worksheet/mc:AlternateContent"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf134769)
{
    createScDoc("xlsx/tdf134769.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    // without the fix in place, the legacyDrawing would have been exported after the checkbox
    // and Excel would have claimed the document is corrupted
    // Use their ids to check the order
    assertXPath(pSheet, "/x:worksheet/x:drawing"_ostr, "id"_ostr, "rId2");
    assertXPath(pSheet, "/x:worksheet/x:legacyDrawing"_ostr, "id"_ostr, "rId3");
    assertXPath(pSheet,
                "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/"
                "mc:Choice/x:control"_ostr,
                "id"_ostr, "rId4");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf106181)
{
    createScDoc("ods/tdf106181.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet,
                "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/"
                "mc:Choice/x:control"_ostr,
                "name"_ostr, "Check Box");
    assertXPath(pSheet,
                "/x:worksheet/mc:AlternateContent/mc:Choice/x:controls/mc:AlternateContent/"
                "mc:Choice/x:control/x:controlPr"_ostr,
                "altText"_ostr, "Check Box 1");

    xmlDocUniquePtr pDrawing = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing);

    assertXPath(
        pDrawing,
        "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr"_ostr,
        "name"_ostr, "Check Box 1");
    assertXPath(
        pDrawing,
        "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr"_ostr,
        "descr"_ostr, "Check Box");
    assertXPath(
        pDrawing,
        "/xdr:wsDr/mc:AlternateContent/mc:Choice/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr"_ostr,
        "hidden"_ostr, "0");

    xmlDocUniquePtr pVmlDrawing = parseExport("xl/drawings/vmlDrawing1.vml");
    CPPUNIT_ASSERT(pVmlDrawing);
    assertXPathContent(pVmlDrawing, "//xx:ClientData/xx:FmlaLink"_ostr, "$D$9");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf145057)
{
    createScDoc("xlsx/tdf145057.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/tables/table1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:colorFilter"_ostr, "dxfId"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf105272)
{
    createScDoc("xlsx/tdf105272.xlsx");
    saveAndReload("Calc Office Open XML");
    ScDocument* pDoc = getScDoc();
    //without the fix in place,it would fail
    //Expected: Table1[[#This Row],[Total]]/Table1[[#This Row],['# Athletes]]
    //Actual  : table1[[#this row],[total]]/table1[[#this row],['# athletes]]

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong formula", OUString("=Table1[[#This Row],[Total]]/Table1[[#This Row],['# Athletes]]"),
        pDoc->GetFormula(7, 3, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf118990)
{
    createScDoc("xlsx/tdf118990.xlsx");
    saveAndReload("Calc Office Open XML");
    ScDocument* pDoc = getScDoc();

    // TODO: also test A1, which contains a UNC reference to \\localhost\share\lookupsource.xlsx,
    // but currently looses "localhost" part when normalized in INetURLObject, becoming
    // file:///share/lookupsource.xlsx - which is incorrect, since it points to local filesystem
    // and not to Windows network share.

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong Windows share (using host IP) URL in A2",
        OUString("=VLOOKUP(B1,'file://192.168.1.1/share/lookupsource.xlsx'#$Sheet1.A1:B5,2)"),
        pDoc->GetFormula(0, 1, 0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong Windows share (using hostname) URL in A3",
        OUString("=VLOOKUP(B1,'file://NETWORKHOST/share/lookupsource.xlsx'#$Sheet1.A1:B5,2)"),
        pDoc->GetFormula(0, 2, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf121612)
{
    createScDoc("ods/tdf121612.ods");
    saveAndReload("Calc Office Open XML");

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

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/pivotCache/pivotCacheDefinition1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:pivotCacheDefinition"_ostr, "recordCount"_ostr, "4");
    assertXPath(pDoc, "//x:pivotCacheDefinition"_ostr, "createdVersion"_ostr, "3");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testXltxExport)
{
    // Create new document
    createScDoc();

    // Export as template and check content type
    save("Calc MS Excel 2007 XML Template");
    xmlDocUniquePtr pDoc = parseExport("[Content_Types].xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/ContentType:Types/ContentType:Override[@PartName='/xl/workbook.xml']"_ostr,
                "ContentType"_ostr,
                "application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testPivotCacheAfterExportXLSX)
{
    createScDoc("ods/numgroup_example.ods");

    // export only
    save("Calc Office Open XML");

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

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[1]"_ostr, "location"_ostr,
                "'1.1.1.1'!C1");
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[2]"_ostr, "location"_ostr,
                "'1.1.1.1'!C2");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf115192XLSX)
{
    createScDoc("xlsx/test_115192.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "TargetMode"_ostr,
                "External");
    assertXPathNoAttribute(pDoc, "/rels:Relationships/rels:Relationship[@Id='rId2']"_ostr,
                           "TargetMode"_ostr);
    assertXPath(pDoc, "/rels:Relationships/rels:Relationship[@Id='rId3']"_ostr, "TargetMode"_ostr,
                "External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf142764)
{
    createScDoc("ods/tdf142764.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:worksheet/x:headerFooter"_ostr, "differentOddEven"_ostr, "true");
    assertXPath(pSheet, "/x:worksheet/x:headerFooter"_ostr, "differentFirst"_ostr, "true");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf91634XLSX)
{
    createScDoc("xlsx/image_hyperlink.xlsx");
    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:pic/xdr:nvPicPr/xdr:cNvPr/a:hlinkClick"_ostr,
                1);

    xmlDocUniquePtr pXmlRels = parseExport("xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                "https://www.google.com/");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr,
                "TargetMode"_ostr, "External");
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
    save("Calc Office Open XML");

    // check validation
    xmlDocUniquePtr pDocXml = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDocXml);
    assertXPathContent(pDocXml, "/x:worksheet/x:dataValidations/x:dataValidation/x:formula1"_ostr,
                       "#REF!");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf115159)
{
    createScDoc("xlsx/tdf115159.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/workbook.xml");
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf112567)
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

    createScDoc("xlsx/tdf112567.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/workbook.xml");
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf75702)
{
    // The problem was that line breaks were not imported.
    constexpr OUString sA1(u"line1\nline2"_ustr);

    createScDoc("ods/tdf75702_textLineBreak.ods");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("load a1", sA1, pDoc->GetString(0, 0, 0));

    saveAndReload("calc8");
    pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("reload a1", sA1, pDoc->GetString(0, 0, 0));

    save("calc8");
    xmlDocUniquePtr pContent = parseExport("content.xml");
    assertXPath(pContent, "//table:table-row[1]/table:table-cell/text:p"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf103829)
{
    // The problem was that tabspaces were not imported or exported at all.
    // These strings match the current implementations of CELLTYPE_EDIT and CELLTYPE_STRING.
    constexpr OUString sA1(u"\x001Leading tab\nTHREE tabs inside: [\x001\x001\x001]"_ustr);
    constexpr OUString sA2(u"\tLeading tab. THREE tabs inside: [\t\t\t]"_ustr);

    createScDoc("ods/tdf103829_textTab.ods");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("load a1", sA1, pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("load a2", sA2, pDoc->GetString(0, 1, 0));

    saveAndReload("calc8");
    pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("reload a1", sA1, pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("reload a2", sA2, pDoc->GetString(0, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf122191)
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

    createScDoc("xlsx/tdf122191.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(OUString("IGAZ"), pDoc->GetString(0, 0, 0));

    saveAndReload("Calc Office Open XML");

    pDoc = getScDoc();
    // Without the fix in place, this test would have failed with
    // - Expected: IGAZ
    // - Actual  : BOOL00AN
    CPPUNIT_ASSERT_EQUAL(OUString("IGAZ"), pDoc->GetString(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf142881)
{
    createScDoc("xlsx/tdf142881.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDrawing1 = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDrawing1);

    // Verify that the shapes are rotated and positioned in the expected way
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:from/xdr:col"_ostr, "11");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:from/xdr:row"_ostr, "0");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:to/xdr:col"_ostr, "12");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:to/xdr:row"_ostr, "19");

    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:from/xdr:col"_ostr, "2");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:from/xdr:row"_ostr, "8");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:to/xdr:col"_ostr, "7");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:to/xdr:row"_ostr, "10");

    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:from/xdr:col"_ostr, "10");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:from/xdr:row"_ostr, "9");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:to/xdr:col"_ostr, "11");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:to/xdr:row"_ostr, "26");

    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:from/xdr:col"_ostr, "2");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:from/xdr:row"_ostr, "17");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:to/xdr:col"_ostr, "8");
    assertXPathContent(pDrawing1, "/xdr:wsDr/xdr:twoCellAnchor[4]/xdr:to/xdr:row"_ostr, "19");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf112567b)
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

    createScDoc("ods/tdf112567.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/workbook.xml");
    CPPUNIT_ASSERT(pDoc);

    //assert the existing OOXML built-in name is not duplicated
    assertXPath(pDoc, "/x:workbook/x:definedNames/x:definedName"_ostr, 1);

    //and it contains "," instead of ";"
    assertXPathContent(pDoc, "/x:workbook/x:definedNames/x:definedName[1]"_ostr,
                       "Sheet1!$A:$A,Sheet1!$1:$1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf123645XLSX)
{
    createScDoc("xlsx/chart_hyperlink.xlsx");
    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:graphicFrame/xdr:nvGraphicFramePr/xdr:cNvPr/"
                "a:hlinkClick"_ostr,
                1);
    assertXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:graphicFrame/xdr:nvGraphicFramePr/xdr:cNvPr/"
                "a:hlinkClick"_ostr,
                1);
    assertXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor[3]/xdr:graphicFrame/xdr:nvGraphicFramePr/xdr:cNvPr/"
                "a:hlinkClick"_ostr,
                1);

    xmlDocUniquePtr pXmlRels = parseExport("xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr,
                "TargetMode"_ostr, "External");
    assertXPathNoAttribute(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId3']"_ostr,
                           "TargetMode"_ostr);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId5']"_ostr,
                "TargetMode"_ostr, "External");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                "file:///C:/TEMP/test.xlsx");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId3']"_ostr, "Target"_ostr,
                "#Sheet2!A1");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId5']"_ostr, "Target"_ostr,
                "https://bugs.documentfoundation.org/show_bug.cgi?id=123645");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf125173XLSX)
{
    createScDoc("ods/text_box_hyperlink.ods");
    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp/xdr:nvSpPr/xdr:cNvPr/a:hlinkClick"_ostr,
                1);

    xmlDocUniquePtr pXmlRels = parseExport("xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                "http://www.google.com/");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr,
                "TargetMode"_ostr, "External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf79972XLSX)
{
    createScDoc("xlsx/tdf79972.xlsx");
    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink"_ostr, "ref"_ostr, "A1");

    xmlDocUniquePtr pXmlRels = parseExport("xl/worksheets/_rels/sheet1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship"_ostr, "Target"_ostr,
                "https://bugs.documentfoundation.org/show_bug.cgi?id=79972");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship"_ostr, "TargetMode"_ostr,
                "External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf126024XLSX)
{
    createScDoc("xlsx/hyperlink_formula.xlsx");
    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink"_ostr, "ref"_ostr, "A2");

    xmlDocUniquePtr pXmlRels = parseExport("xl/worksheets/_rels/sheet1.xml.rels");
    CPPUNIT_ASSERT(pXmlRels);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship"_ostr, "Target"_ostr,
                "https://bugs.documentfoundation.org/");
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship"_ostr, "TargetMode"_ostr,
                "External");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testProtectedRange)
{
    createScDoc("xlsx/protectedRange.xlsx");
    save("Calc Office Open XML");

    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    // entire row was being exported as shorthand 'B:B' which LO couldn't read back
    assertXPath(pDoc, "//x:protectedRanges/x:protectedRange"_ostr, "sqref"_ostr, "B1:B1048576");
}

CPPUNIT_TEST_FIXTURE(ScExportTest2, testTdf91332)
{
    createScDoc("xlsx/tdf91332.xlsx");
    saveAndReload("Calc Office Open XML");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShapeProps(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: rgba[90cf47ff]
    // - Actual  : rgba[ffffffff]
    // i.e. fill color inherited from theme lost after export.
    Color nColor;
    xShapeProps->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x90cf47), nColor);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
