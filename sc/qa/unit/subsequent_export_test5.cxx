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
#include <scitems.hxx>
#include <attrib.hxx>
#include <stlpool.hxx>
#include <formulacell.hxx>
#include <postit.hxx>
#include <tabprotection.hxx>
#include <dpobject.hxx>

#include <editeng/wghtitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <comphelper/propertyvalue.hxx>
#include <test/commontesttools.hxx>
#include <tools/UnitConversion.hxx>
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
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScExportTest5 : public ScModelTestBase
{
public:
    ScExportTest5();
};

ScExportTest5::ScExportTest5()
    : ScModelTestBase(u"sc/qa/unit/data"_ustr)
{
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf142929_filterLessThanXLSX)
{
    // Document contains a standard filter with '<' condition.
    createScDoc("xlsx/tdf142929.xlsx");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "//x:customFilters/x:customFilter", "val", u"2");
    assertXPath(pDoc, "//x:customFilters/x:customFilter", "operator", u"lessThan");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testInvalidNamedRange)
{
    // Given a document which has a named range (myname) that refers to the "1" external link, but
    // the link's type is xlPathMissing, when importing that document:
    createScDoc("xlsx/invalid-named-range.xlsx");

    // Then make sure that named range is ignored, as "1" can't be resolved, and exporting it back
    // to XLSX (without the xlPathMissing link) would corrupt the document:
    uno::Reference<beans::XPropertySet> xDocProps(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNamedRanges(
        xDocProps->getPropertyValue(u"NamedRanges"_ustr), uno::UNO_QUERY);
    // Without the fix in place, this test would have failed, we didn't ignore the problematic named
    // range on import.
    CPPUNIT_ASSERT(!xNamedRanges->hasByName(u"myname"_ustr));
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testExternalDefinedNameXLSX)
{
    createScDoc("xlsx/tdf144397.xlsx");
    saveAndReload(TestFilter::XLSX);

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->ReloadAllLinks();
    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    // "January"
    {
        const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 1, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::String, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(u"January"_ustr, aRes.maString.getString());
    }
    // "March"
    {
        const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 3, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::String, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(u"March"_ustr, aRes.maString.getString());
    }
    // "Empty = #N/A"
    {
        const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 5, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::Error, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(u""_ustr, aRes.maString.getString());
    }
    // "June"
    {
        const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 6, 0));
        sc::FormulaResultValue aRes = pFC->GetResult();
        CPPUNIT_ASSERT_EQUAL(sc::FormulaResultValue::String, aRes.meType);
        CPPUNIT_ASSERT_EQUAL(u"June"_ustr, aRes.maString.getString());
    }

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDocXml = parseExport(u"xl/externalLinks/externalLink1.xml"_ustr);

    CPPUNIT_ASSERT(pDocXml);
    assertXPath(pDocXml, "/x:externalLink/x:externalBook/x:sheetNames/x:sheetName", "val",
                u"Munka1");
    assertXPath(pDocXml, "/x:externalLink/x:externalBook/x:definedNames/x:definedName", "name",
                u"MonthNames");
    // TODO: no need for the [1] external document identifier
    assertXPath(pDocXml, "/x:externalLink/x:externalBook/x:definedNames/x:definedName", "refersTo",
                u"[1]Munka1!$A$2:$A$13");
    assertXPath(pDocXml, "/x:externalLink/x:externalBook/x:sheetDataSet/x:sheetData", "sheetId",
                u"0");
    assertXPath(pDocXml, "/x:externalLink/x:externalBook/x:sheetDataSet/x:sheetData/x:row[2]", "r",
                u"3");
    assertXPathContent(
        pDocXml, "/x:externalLink/x:externalBook/x:sheetDataSet/x:sheetData/x:row[2]/x:cell/x:v",
        u"February");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testHyperlinkLocationXLSX)
{
    createScDoc("ods/tdf143220.ods");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pDoc);

    // tdf#143220 link to sheet not valid without cell reference
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[@ref='A1']", "location", u"Sheet2!A1");

    // tdf#145079 link with defined name target didn't work because Calc added "A1" at the end
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[@ref='A2']", "location", u"name");
    assertXPath(pDoc, "/x:worksheet/x:hyperlinks/x:hyperlink[@ref='A3']", "location", u"db");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf142264ManyChartsToXLSX)
{
    // The cache size for the test should be small enough, to make sure that some charts get
    // unloaded in the process, and then loaded on demand properly (default is currently 200)
    ScopedConfigValue<officecfg::Office::Common::Cache::DrawingEngine::OLE_Objects> aCfg(20);

    createScDoc("ods/many_charts.ods");
    saveAndReload(TestFilter::XLSX);

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
        css::uno::Reference<css::chart2::XChartDocument> xChart(
            xProps->getPropertyValue(u"Model"_ustr), css::uno::UNO_QUERY_THROW);
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
        css::uno::Reference<css::chart2::XChartDocument> xChart(
            xProps->getPropertyValue(u"Model"_ustr), css::uno::UNO_QUERY_THROW);
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

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf143929MultiColumnToODS)
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
        css::uno::Reference<css::text::XTextColumns> xCols(
            xProps->getPropertyValue(u"TextColumns"_ustr), css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCols->getColumnCount());
        css::uno::Reference<css::beans::XPropertySet> xColProps(xCols, css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(css::uno::Any(sal_Int32(1000)),
                             xColProps->getPropertyValue(u"AutomaticDistance"_ustr));
    }

    saveAndReload(TestFilter::ODS);
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
        css::uno::Reference<css::text::XTextColumns> xCols(
            xProps->getPropertyValue(u"TextColumns"_ustr), css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCols->getColumnCount());
        css::uno::Reference<css::beans::XPropertySet> xColProps(xCols, css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(css::uno::Any(sal_Int32(1000)),
                             xColProps->getPropertyValue(u"AutomaticDistance"_ustr));
    }

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
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
        "column-count", u"2");
    // Only test that "column-gap" attribute exists, not its value that depends on locale (cm, in)
    getXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[@style:family='graphic']/"
        "style:graphic-properties/style:columns",
        "column-gap");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf142578)
{
    createScDoc("ods/tdf142578.ods");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
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
    xmlDocUniquePtr pStyles = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pStyles);

    OString sDxfColorFilterXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfIdColorFilter)
                                 + "]/x:fill/x:patternFill/x:fgColor");
    assertXPath(pStyles, sDxfColorFilterXPath, "rgb", u"FF81D41A");

    OString sDxfCondFormatXPath("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfIdCondFormat)
                                + "]/x:fill/x:patternFill/x:bgColor");
    assertXPath(pStyles, sDxfCondFormatXPath, "rgb", u"FFFFCCCC");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf145059)
{
    createScDoc("ods/tdf145059.ods");

    // Export to xlsx.
    save(TestFilter::XLSX);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);
    xmlDocUniquePtr pStyle = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pStyle);

    sal_Int32 nColorFilterDxdId
        = getXPath(pSheet, "/x:worksheet/x:autoFilter/x:filterColumn/x:colorFilter", "dxfId")
              .toInt32();

    // Ensure that dxf id is not -1
    CPPUNIT_ASSERT(nColorFilterDxdId >= 0);

    // Find color by this dxfid
    OString sDxfIdPath = "/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nColorFilterDxdId + 1)
                         + "]/x:fill/x:patternFill/x:fgColor";
    assertXPath(pStyle, sDxfIdPath, "rgb", u"FF4472C4");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf130104_XLSXIndent)
{
    createScDoc("xlsx/tdf130104_indent.xlsx");

    // Resave the xlsx file without any modification.
    save(TestFilter::XLSX);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);
    xmlDocUniquePtr pStyle = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pStyle);

    // Check to see whether the indents remain the same as the original ones:

    // Get the style index number for cell A1
    sal_Int32 nCellA1StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[1]/x:c[1]", "s").toInt32() + 1;
    // The indent for cell A1 should be 0
    OString sStyleA1XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA1StyleIndex) + "]/x:alignment";
    // (if this assertion fails, you should first check whether there is no style index set for this cell)
    assertXPath(pStyle, sStyleA1XPath, "indent", u"0");

    sal_Int32 nCellA3StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[3]/x:c[1]", "s").toInt32() + 1;
    // The indent for cell A3 should be 1
    OString sStyleA3XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA3StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA3XPath, "indent", u"1");

    sal_Int32 nCellA6StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[6]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA6XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA6StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA6XPath, "indent", u"2");

    sal_Int32 nCellA9StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[9]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA9XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA9StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA9XPath, "indent", u"3");

    sal_Int32 nCellA12StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[12]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA12XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA12StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA12XPath, "indent", u"4");

    sal_Int32 nCellA15StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[15]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA15XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA15StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA15XPath, "indent", u"5");

    sal_Int32 nCellA18StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[18]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA18XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA18StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA18XPath, "indent", u"6");

    sal_Int32 nCellA21StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[21]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA21XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA21StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA21XPath, "indent", u"7");

    sal_Int32 nCellA24StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[24]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA24XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA24StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA24XPath, "indent", u"8");

    sal_Int32 nCellA27StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[27]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA27XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA27StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA27XPath, "indent", u"9");

    sal_Int32 nCellA30StyleIndex
        = getXPath(pSheet, "/x:worksheet/x:sheetData/x:row[30]/x:c[1]", "s").toInt32() + 1;
    OString sStyleA30XPath
        = "/x:styleSheet/x:cellXfs/x:xf[" + OString::number(nCellA30StyleIndex) + "]/x:alignment";
    assertXPath(pStyle, sStyleA30XPath, "indent", u"10");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testWholeRowBold)
{
    createScDoc();

    {
        ScDocument* pDoc = getScDoc();

        // Make entire second row bold.
        ScPatternAttr boldAttr(pDoc->getCellAttributeHelper());
        boldAttr.ItemSetPut(SvxWeightItem(WEIGHT_BOLD, ATTR_FONT_WEIGHT));
        pDoc->ApplyPatternAreaTab(0, 1, pDoc->MaxCol(), 1, 0, boldAttr);
    }

    saveAndReload(TestFilter::ODS);
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(SCCOL(INITIALCOLCOUNT), pDoc->GetAllocatedColumnsCount(0));
    vcl::Font aFont;
    pDoc->GetPattern(pDoc->MaxCol(), 1, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD,
                                 aFont.GetWeightMaybeAskConfig());

    saveAndReload(TestFilter::XLSX);
    pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(SCCOL(INITIALCOLCOUNT), pDoc->GetAllocatedColumnsCount(0));
    pDoc->GetPattern(pDoc->MaxCol(), 1, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD,
                                 aFont.GetWeightMaybeAskConfig());
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testXlsxRowsOrder)
{
    createScDoc("xlsx/tdf58243.xlsx");
    // Make sure code in SheetDataBuffer doesn't assert columns/rows sorting.
    save(TestFilter::XLSX);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf91286)
{
    createScDoc("ods/tdf91286.ods");
    save(TestFilter::XLSX);

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

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf148820)
{
    createScDoc("xlsx/tdf148820.xlsx");
    save(TestFilter::XLSX);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    CPPUNIT_ASSERT_EQUAL(u"20"_ustr,
                         getXPathContent(pSheet, "count(/x:worksheet/x:conditionalFormatting)"));
    CPPUNIT_ASSERT_EQUAL(
        u"20"_ustr,
        getXPathContent(pSheet, "count(/x:worksheet/x:conditionalFormatting/x:cfRule)"));
    sal_Int32 nDxfIdCondFormatFirst
        = getXPath(pSheet, "/x:worksheet/x:conditionalFormatting[1]/x:cfRule", "dxfId").toInt32()
          + 1;
    sal_Int32 nDxfIdCondFormatLast
        = getXPath(pSheet, "/x:worksheet/x:conditionalFormatting[5]/x:cfRule", "dxfId").toInt32()
          + 1;

    xmlDocUniquePtr pStyles = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pStyles);

    OString sDxfCondFormatXPath("/x:styleSheet/x:dxfs/x:dxf["
                                + OString::number(nDxfIdCondFormatFirst)
                                + "]/x:fill/x:patternFill/x:bgColor");
    assertXPath(pStyles, sDxfCondFormatXPath, "rgb", u"FF53B5A9");
    sDxfCondFormatXPath
        = OString("/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nDxfIdCondFormatLast)
                  + "]/x:fill/x:patternFill/x:bgColor");
    assertXPath(pStyles, sDxfCondFormatXPath, "rgb", u"FFA30000");
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

CPPUNIT_TEST_FIXTURE(ScExportTest5, testBlankInExponent)
{
    createScDoc("ods/tdf156449-Blank-In-Exponent.ods");

    // save to ODS and reload
    saveAndReload(TestFilter::ODS);
    lcl_TestNumberFormat(*getScDoc(), u"0.00E+?0"_ustr);
    lcl_SetNumberFormat(*getScDoc(), u"0.00E+??"_ustr);
    // at least one '0' in exponent
    saveAndReload(TestFilter::ODS);
    lcl_TestNumberFormat(*getScDoc(), u"0.00E+?0"_ustr);

    // save to XLSX and reload
    saveAndReload(TestFilter::XLSX);
    lcl_TestNumberFormat(*getScDoc(), u"0.00E+?0"_ustr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testSecondsWithoutTruncateAndDecimals)
{
    createScDoc("xlsx/seconds-without-truncate-and-decimals.xlsx");
    lcl_TestNumberFormat(*getScDoc(), u"[SS].00"_ustr);

    // save to ODS and reload
    saveAndReload(TestFilter::ODS);
    lcl_TestNumberFormat(*getScDoc(), u"[SS].00"_ustr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf152724_BlankWidthCharacter_ODS)
{
    createScDoc("ods/tdf152724-Blank-width-char.ods");

    // save to ODS and reload
    saveAndReload(TestFilter::ODS);
    lcl_TestNumberFormat(*getScDoc(), u"[>0]_-?0;[<0]-?0;_-?0;@"_ustr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf152724_BlankWidthCharacter_XLSX)
{
    createScDoc("ods/tdf152724-Blank-width-char.ods");

    // save to XLSX and reload
    saveAndReload(TestFilter::XLSX);
    lcl_TestNumberFormat(*getScDoc(), u"_-?0;-?0;_-?0;@"_ustr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf170670_BlankWidthCharacter_ODS)
{
    createScDoc("ods/tdf170670-Blank-width-char.ods");

    // save to ODS and reload
    saveAndReload(TestFilter::ODS);
    lcl_TestNumberFormat(*getScDoc(), u"[>0]#,##0_);[<0](#,##0);\"-\"_)"_ustr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf170670_BlankWidthCharacter_XLSX)
{
    createScDoc("ods/tdf170670-Blank-width-char.ods");

    // save to XLSX and reload
    saveAndReload(TestFilter::XLSX);
    lcl_TestNumberFormat(*getScDoc(), u"#,##0_);\\(#,##0\\);-_)"_ustr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testEmbeddedTextInDecimal)
{
    createScDoc("xlsx/embedded-text-in-decimal.xlsx");
    lcl_TestNumberFormat(*getScDoc(), u"#,##0.000\" \"###\" \"###"_ustr);

    // save to ODS and reload
    saveAndReload(TestFilter::ODS);
    lcl_TestNumberFormat(*getScDoc(), u"#,##0.000\" \"###\" \"###"_ustr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testLowercaseExponent)
{
    createScDoc("ods/tdf153993-Exponent-lower-case.ods");

    // save to ODS and reload
    saveAndReload(TestFilter::ODS);
    lcl_TestNumberFormat(*getScDoc(), u"0.000\" \"000\" \"e+\" \"0"_ustr);

    // save to XLSX and reload
    // lower case not preserve in XLSX
    saveAndReload(TestFilter::XLSX);
    lcl_TestNumberFormat(*getScDoc(), u"0.000 000 E+ 0"_ustr);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTotalsRowFunction)
{
    createScDoc("xlsx/totalsRowFunction.xlsx");
    saveAndReload(TestFilter::XLSX);
    {
        xmlDocUniquePtr pDocXml = parseExport(u"xl/tables/table1.xml"_ustr);
        CPPUNIT_ASSERT(pDocXml);
        assertXPath(pDocXml, "/x:table/x:tableColumns/x:tableColumn[5]", "totalsRowFunction",
                    u"sum");
    }
    ScDocument* pDoc = getScDoc();
    pDoc->InsertCol(ScRange(3, 0, 0, 3, pDoc->MaxRow(), 0)); // Insert col 4
    saveAndReload(TestFilter::XLSX);
    {
        xmlDocUniquePtr pDocXml = parseExport(u"xl/tables/table1.xml"_ustr);
        CPPUNIT_ASSERT(pDocXml);
        assertXPathNoAttribute(pDocXml, "/x:table/x:tableColumns/x:tableColumn[5]",
                               "totalsRowFunction");
        assertXPath(pDocXml, "/x:table/x:tableColumns/x:tableColumn[6]", "totalsRowFunction",
                    u"sum");
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTotalsRowShown)
{
    createScDoc("xlsx/totalsRowShown.xlsx");
    saveAndReload(TestFilter::XLSX);
    {
        xmlDocUniquePtr pDocXml = parseExport(u"xl/tables/table1.xml"_ustr);
        CPPUNIT_ASSERT(pDocXml);
        assertXPathNoAttribute(pDocXml, "/x:table", "totalsRowShown");
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf167689_xmlMaps_and_xmlColumnPr)
{
    createScDoc("xlsx/tdf167689_xmlMaps_and_xmlColumnPr.xlsx");
    save(TestFilter::XLSX);

    // xl/xmlMaps.xml
    xmlDocUniquePtr pDocXml = parseExport(u"xl/xmlMaps.xml"_ustr);
    CPPUNIT_ASSERT(pDocXml);

    assertXPath(pDocXml,
                "/x:MapInfo/Schema/xsd:schema/xsd:element/xsd:complexType/xsd:sequence/xsd:element/"
                "xsd:complexType/xsd:sequence/xsd:element[3]/xsd:complexType/xsd:sequence/"
                "xsd:element[1]",
                "name", u"Code");

    // xl/tables/table1.xml
    xmlDocUniquePtr pDocXmlTables = parseExport(u"xl/tables/table1.xml"_ustr);
    CPPUNIT_ASSERT(pDocXmlTables);

    // test xpath attribute of <xmlColumnPr>
    assertXPath(pDocXmlTables, "/x:table/x:tableColumns/x:tableColumn[1]/x:xmlColumnPr", "xpath",
                u"/DataList/TransactionTypeList/TransactionType/Code");

    // test tableType attribute of <table>
    assertXPath(pDocXmlTables, "/x:table", "tableType", u"xml");

    // test uniqueName attribute of <tableColumn>
    assertXPath(pDocXmlTables, "/x:table/x:tableColumns/x:tableColumn[2]", "uniqueName",
                u"Description");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf167689_tableType)
{
    createScDoc("xlsx/tdf167689_tableType.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pDocXmlTables = parseExport(u"xl/tables/table1.xml"_ustr);
    CPPUNIT_ASSERT(pDocXmlTables);

    // if the source document does not have tableType attribute in <table> node,
    // it shouldn't exist in the exported document either.
    assertXPathNoAttribute(pDocXmlTables, "/x:table", "tableType");

    // if there is no tableType, there should not be any uniqueName attribute in <tableColumn>
    assertXPathNoAttribute(pDocXmlTables, "/x:table/x:tableColumns/x:tableColumn[1]", "uniqueName");
    assertXPathNoAttribute(pDocXmlTables, "/x:table/x:tableColumns/x:tableColumn[2]", "uniqueName");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testAutofilterHiddenButton)
{
    createScDoc("xlsx/hiddenButton.xlsx");
    saveAndReload(TestFilter::XLSX);
    xmlDocUniquePtr pDocXml = parseExport(u"xl/tables/table1.xml"_ustr);
    CPPUNIT_ASSERT(pDocXml);
    for (int i = 1; i <= 5; i++)
    {
        OString sPath
            = OString::Concat("/x:table/x:autoFilter/x:filterColumn[") + OString::number(i) + "]";
        assertXPath(pDocXml, sPath, "hiddenButton", u"1");
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testShapeStyles)
{
    createScDoc();

    {
        uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                             uno::UNO_QUERY_THROW);
        uno::Reference<container::XNameContainer> xGraphicStyles(
            xStyleFamiliesSupplier->getStyleFamilies()->getByName(u"GraphicStyles"_ustr),
            uno::UNO_QUERY_THROW);

        // create styles
        uno::Reference<style::XStyle> xStyle(
            xMSF->createInstance(u"com.sun.star.style.GraphicStyle"_ustr), uno::UNO_QUERY_THROW);
        xGraphicStyles->insertByName(u"MyStyle1"_ustr, Any(xStyle));
        uno::Reference<beans::XPropertySet> xPropertySet(xStyle, uno::UNO_QUERY_THROW);
        xPropertySet->setPropertyValue(u"FillColor"_ustr, Any(COL_RED));
        xPropertySet->setPropertyValue(u"FillTransparence"_ustr, Any(sal_Int16(40)));

        xStyle.set(xMSF->createInstance(u"com.sun.star.style.GraphicStyle"_ustr),
                   uno::UNO_QUERY_THROW);
        xGraphicStyles->insertByName(u"MyStyle2"_ustr, Any(xStyle));
        xStyle->setParentStyle(u"MyStyle1"_ustr);

        xStyle.set(xMSF->createInstance(u"com.sun.star.style.GraphicStyle"_ustr),
                   uno::UNO_QUERY_THROW);
        xGraphicStyles->insertByName(u"MyStyle3"_ustr, Any(xStyle));
        xStyle->setParentStyle(u"MyStyle2"_ustr);

        // create shape
        uno::Reference<drawing::XShape> xShape(
            xMSF->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr),
            uno::UNO_QUERY_THROW);

        uno::Reference<drawing::XDrawPagesSupplier> xDPS(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShapes> xShapes(xDPS->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY_THROW);
        xShapes->add(xShape);
        uno::Reference<beans::XPropertySet>(xShape, uno::UNO_QUERY_THROW)
            ->setPropertyValue(u"Style"_ustr, Any(xStyle));
    }

    saveAndReload(TestFilter::ODS);

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDPS(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShapes> xShapes(xDPS->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xShape(xShapes->getByIndex(0), uno::UNO_QUERY_THROW);

        // check style hierarchy
        uno::Reference<style::XStyle> xStyle(xShape->getPropertyValue(u"Style"_ustr),
                                             uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"MyStyle3"_ustr, xStyle->getName());
        CPPUNIT_ASSERT_EQUAL(u"MyStyle2"_ustr, xStyle->getParentStyle());

        // check that styles have effect on shapes
        Color nColor;
        xShape->getPropertyValue(u"FillColor"_ustr) >>= nColor;
        CPPUNIT_ASSERT_EQUAL(COL_RED, nColor);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(40),
                             xShape->getPropertyValue(u"FillTransparence"_ustr).get<sal_Int16>());
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testCommentStyles)
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

        auto pStyleSheet
            = &pDoc->GetStyleSheetPool()->Make(u"MyStyle1"_ustr, SfxStyleFamily::Frame);
        auto& rSet = pStyleSheet->GetItemSet();
        rSet.Put(SvxFontHeightItem(1129, 100, EE_CHAR_FONTHEIGHT));

        pCaption->SetStyleSheet(static_cast<SfxStyleSheet*>(pStyleSheet), false);

        // Hidden comments use different code path on import
        pNote->ShowCaption(aPos, false);
    }

    saveAndReload(TestFilter::ODS);

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
        CPPUNIT_ASSERT_EQUAL(u"MyStyle1"_ustr, pCaption->GetStyleSheet()->GetName());
    }

    saveAndReload(TestFilter::XLSX);

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

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf119565)
{
    createScDoc("xlsx/tdf119565.xlsx");
    saveAndReload(TestFilter::XLSX);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShapeProps(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 35
    // - Actual  : 0
    // i.e. line width inherited from theme lost after export.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(35),
                         xShapeProps->getPropertyValue(u"LineWidth"_ustr).get<sal_Int32>());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 4
    // i.e. line joint inherited from theme lost after export.
    CPPUNIT_ASSERT_EQUAL(
        drawing::LineJoint_MITER,
        xShapeProps->getPropertyValue(u"LineJoint"_ustr).get<drawing::LineJoint>());
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf152980)
{
    createScDoc("csv/tdf152980.csv");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    saveAndReload(TestFilter::XLSX);
    pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    // - Expected: The part between a and b does not change
    // - Actual  : Only the characters a and b remain
    CPPUNIT_ASSERT_EQUAL(u"a_x1_b"_ustr, pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"a_x01_b"_ustr, pDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"a_x001_b"_ustr, pDoc->GetString(0, 2, 0));

    // The character code does not change in both cases
    CPPUNIT_ASSERT_EQUAL(u"a_x0001_b"_ustr, pDoc->GetString(0, 3, 0));

    // The escape characters are handled correctly in both cases
    CPPUNIT_ASSERT_EQUAL(u"a_xfoo\nb"_ustr, pDoc->GetString(0, 4, 0));
    CPPUNIT_ASSERT_EQUAL(u"a\tb"_ustr, pDoc->GetString(0, 5, 0));
    CPPUNIT_ASSERT_EQUAL(u"a\nb"_ustr, pDoc->GetString(0, 6, 0));
    CPPUNIT_ASSERT_EQUAL(u"a\n\nb"_ustr, pDoc->GetString(0, 7, 0));

    // LO doesn't require "wrap text" to display multiline content. Excel does.
    // tdf#161453: ensure A8 was set to wrap text, so Excel doesn't display as single line
    SCTAB nTab = 0;
    SCROW nRow = 7;
    CPPUNIT_ASSERT(pDoc->GetAttr(0, nRow, nTab, ATTR_LINEBREAK).GetValue());
    // Without the fix, this was a single line high (446). It should be 3 lines high (1236).
    int nHeight = convertTwipToMm100(pDoc->GetRowHeight(nRow, nTab, false));
    CPPUNIT_ASSERT_GREATER(1000, nHeight);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf100034)
{
    createScDoc("xlsx/tdf100034.xlsx");
    ScDocument* pDoc = getScDoc();

    // Clear print ranges (Format - Print Ranges - Clear)
    dispatchCommand(mxComponent, u".uno:DeletePrintArea"_ustr, {});

    // Save and load back
    saveAndReload(TestFilter::XLSX);

    // Check if the same print ranges are present
    pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), pDoc->GetPrintRangeCount(0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf157318)
{
    // This document has 2 named ranges; Test1 is global; Test2 is linked to Sheet1)
    createScDoc("ods/tdf157318.ods");
    ScDocument* pDoc = getScDoc();

    // Save as XLSX and load back
    saveAndReload(TestFilter::XLSX);
    pDoc = getScDoc();

    // Check if there is one global named range
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1),
                         static_cast<sal_uInt16>(pDoc->GetRangeName()->size()));

    // Check if there is one named range in the first sheet
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1),
                         static_cast<sal_uInt16>(pDoc->GetRangeName(0)->size()));
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testChangesAuthorDate)
{
    createScDoc("ods/change-tracking.ods");

    // Remove all personal info
    ScopedConfigValue<officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving>
        aCfg(true);

    save(TestFilter::ODS);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc,
                       "/office:document-content/office:body/office:spreadsheet/"
                       "table:tracked-changes/table:cell-content-change[1]/office:change-info/"
                       "dc:creator",
                       u"Author1");
    assertXPathContent(pXmlDoc,
                       "/office:document-content/office:body/office:spreadsheet/"
                       "table:tracked-changes/table:cell-content-change[1]/office:change-info/"
                       "dc:date",
                       u"1970-01-01T12:00:00");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testChangesAuthorDateXLSX)
{
    createScDoc("xlsx/change-tracking.xlsx");

    // Remove all personal info
    ScopedConfigValue<officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving>
        aCfg(true);

    save(TestFilter::XLSX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/revisions/revisionHeaders.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/x:headers/x:header[1]", "userName", u"Author1");
    assertXPath(pXmlDoc, "/x:headers/x:header[1]", "dateTime", u"1970-01-01T12:00:00.000000000Z");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf163554)
{
    createScDoc("xlsx/tdf163554.xlsx");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: =SUM($'time (misc) - last'.B1:$'time (pnrst)'.B1)
    // - Actual  : =SUM('time (pnrst)':$'time (misc) - last'.B1:B1)
    CPPUNIT_ASSERT_EQUAL(u"=SUM($'time (misc) - last'.B1:$'time (pnrst)'.B1)"_ustr,
                         pDoc->GetFormula(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));

    saveAndReload(TestFilter::XLSX);
    pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"=SUM($'time (misc) - last'.B1:$'time (pnrst)'.B1)"_ustr,
                         pDoc->GetFormula(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testNotesAuthor)
{
    createScDoc("xlsx/cell-note.xlsx");

    // Remove all personal info
    ScopedConfigValue<officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving>
        aCfg(true);

    save(TestFilter::XLSX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/comments1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "/x:comments/x:authors/x:author", u"Author1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testSheetProtections)
{
    auto verify = [this]() {
        ScDocument* pDoc = getScDoc();

        // 1. tab autofilter allowed, pivot tables not allowed
        const ScTableProtection* pTab1Protect = pDoc->GetTabProtection(0);
        CPPUNIT_ASSERT(pTab1Protect);
        CPPUNIT_ASSERT(pTab1Protect->isOptionEnabled(ScTableProtection::AUTOFILTER));
        CPPUNIT_ASSERT(!pTab1Protect->isOptionEnabled(ScTableProtection::PIVOT_TABLES));

        // 2. tab autofilter NOT allowed, pivot tables allowed
        const ScTableProtection* pTab2Protect = pDoc->GetTabProtection(1);
        CPPUNIT_ASSERT(pTab2Protect);
        CPPUNIT_ASSERT(!pTab2Protect->isOptionEnabled(ScTableProtection::AUTOFILTER));
        CPPUNIT_ASSERT(pTab2Protect->isOptionEnabled(ScTableProtection::PIVOT_TABLES));

        // check we have pivot table
        ScDPObject* pDPObj1 = pDoc->GetDPAtCursor(0, 0, 1);
        CPPUNIT_ASSERT(pDPObj1);
        CPPUNIT_ASSERT(!pDPObj1->GetName().isEmpty());

        // 3. tab autofilter NOT allowed, pivot tables not allowed
        const ScTableProtection* pTab3Protect = pDoc->GetTabProtection(2);
        CPPUNIT_ASSERT(pTab3Protect);
        CPPUNIT_ASSERT(!pTab3Protect->isOptionEnabled(ScTableProtection::AUTOFILTER));
        CPPUNIT_ASSERT(!pTab3Protect->isOptionEnabled(ScTableProtection::PIVOT_TABLES));

        // 4. tab autofilter allowed, pivot tables not allowed
        const ScTableProtection* pTab4Protect = pDoc->GetTabProtection(3);
        CPPUNIT_ASSERT(pTab4Protect);
        CPPUNIT_ASSERT(pTab4Protect->isOptionEnabled(ScTableProtection::AUTOFILTER));
        CPPUNIT_ASSERT(!pTab4Protect->isOptionEnabled(ScTableProtection::PIVOT_TABLES));

        // check we have pivot table
        ScDPObject* pDPObj2 = pDoc->GetDPAtCursor(0, 0, 3);
        CPPUNIT_ASSERT(pDPObj2);
        CPPUNIT_ASSERT(!pDPObj2->GetName().isEmpty());
    };

    createScDoc("xlsx/tdfSheetProts.xlsx");
    verify();

    saveAndReload(TestFilter::XLSX);
    verify();

    saveAndReload(TestFilter::ODS);
    verify();
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf164417)
{
    createScDoc("xlsx/tdf164417.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet1 = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet1);

    CPPUNIT_ASSERT_EQUAL(
        0, getXPathPosition(pSheet1, "//x:autoFilter/x:filterColumn/x:filters", "filter"));
    CPPUNIT_ASSERT_EQUAL(
        1, getXPathPosition(pSheet1, "//x:autoFilter/x:filterColumn/x:filters", "dateGroupItem"));
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testOrderOfCNumFmtElements)
{
    createScDoc("xlsx/orderOfCNumFmtElements.xlsx");

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 10
    // - validation error in OOXML export: Errors: 10
    saveAndReload(TestFilter::XLSX);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf165503)
{
    createScDoc("xlsx/tdf165503.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pChart1 = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pChart1);

    // Without the fix in place, this test would have failed with
    // - Expected: 44199
    // - Actual  : 1/3/2021
    // The textual date output can depend on locale, but it'll differ from expected value either way
    assertXPathContent(pChart1,
                       "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:cat/c:numRef/"
                       "c:numCache/c:pt[@idx=\"0\"]/c:v",
                       u"44199");
    // And similarly
    assertXPathContent(pChart1,
                       "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:cat/c:numRef/"
                       "c:numCache/c:pt[@idx=\"4\"]/c:v",
                       u"44844");

    // There should be no node with idx 5 (cell is empty)
    const int aNodes = countXPathNodes(
        pChart1, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:cat/c:numRef/"
                 "c:numCache/c:pt[@idx=\"5\"]");
    CPPUNIT_ASSERT_EQUAL(0, aNodes);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf165655)
{
    createScDoc("xlsx/tdf165655.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pDrawing = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDrawing);

    // Original has 3 drawingML and 1 VML objects
    // Not sure if the VML dropdown should be exported, but as long as it cannot be
    //  exported properly, it should not be exported at all (only the 3 drawingMLs)
    const int aNodes = countXPathNodes(pDrawing, "/xdr:wsDr/xdr:twoCellAnchor");
    CPPUNIT_ASSERT_EQUAL(3, aNodes);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf165886)
{
    createScDoc("xlsx/tdf165886.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[1]/x:c[1]/x:f", u"“");
    // Without the accompanying fix in place, this test would have failed with
    // - Expected: OR(D1=0,D1<>““)
    // - Actual  : OR(D1=0,D1<>““))
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[1]/x:c[2]/x:f", u"OR(D1=0,D1<>““)");
    // Similarly
    // - Expected: OR(E1=0,E1<>“)
    // - Actual  : OR(E1=0,E1<>“))
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[1]/x:c[3]/x:f", u"OR(E1=0,E1<>“)");
    // Similarly
    // - Expected: OR(D2=0,D2<>””)
    // - Actual  : OR(D2=0,D2<>””))
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[2]/x:c[2]/x:f", u"OR(D2=0,D2<>””)");
    // Similarly
    // - Expected: OR(D3=0,D3<>‘‘)
    // - Actual  : OR(D3=0,D3<>‘‘))
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[3]/x:c[2]/x:f", u"OR(D3=0,D3<>‘‘)");
    // Similarly
    // - Expected: OR(D4=0,D4<>’’)
    // - Actual  : OR(D4=0,D4<>’’))
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[4]/x:c[2]/x:f", u"OR(D4=0,D4<>’’)");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf166413)
{
    createScDoc("xlsx/tdf166413.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // Without the accompanying fix in place, this test would have failed with
    // - Expected: NOT(ISERROR(SEARCH("""ABC""",A1)))
    // - Actual  : NOT(ISERROR(SEARCH(""ABC"",A1)))
    assertXPathContent(pSheet,
                       "/x:worksheet/x:conditionalFormatting[@sqref=\"A1:C1\"]/x:cfRule/x:formula",
                       u"NOT(ISERROR(SEARCH(\"\"\"ABC\"\"\",A1)))");
    // Similarly
    // - Expected: ISERROR(SEARCH("""ABC""",A2))
    // - Actual  : ISERROR(SEARCH(""ABC"",A2))
    assertXPathContent(pSheet,
                       "/x:worksheet/x:conditionalFormatting[@sqref=\"A2:C2\"]/x:cfRule/x:formula",
                       u"ISERROR(SEARCH(\"\"\"ABC\"\"\",A2))");
    // Similarly
    // - Expected: LEFT(A3,LEN("""ABC"""))="""ABC"""
    // - Actual  : LEFT(A3,LEN(""ABC""))=""ABC""
    assertXPathContent(pSheet,
                       "/x:worksheet/x:conditionalFormatting[@sqref=\"A3:C3\"]/x:cfRule/x:formula",
                       u"LEFT(A3,LEN(\"\"\"ABC\"\"\"))=\"\"\"ABC\"\"\"");
    // Similarly
    // - Expected: RIGHT(A4,LEN("""ABC"""))="""ABC"""
    // - Actual  : RIGHT(A4,LEN(""ABC""))=""ABC""
    assertXPathContent(pSheet,
                       "/x:worksheet/x:conditionalFormatting[@sqref=\"A4:C4\"]/x:cfRule/x:formula",
                       u"RIGHT(A4,LEN(\"\"\"ABC\"\"\"))=\"\"\"ABC\"\"\"");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf166712)
{
    createScDoc("xlsx/tdf166712.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pConn = parseExport(u"xl/connections.xml"_ustr);
    CPPUNIT_ASSERT(pConn);

    // empty dbPr/olapPr mustn't exist in the result's xl/connections.xml
    assertXPath(pConn, "/x:connections/x:connection/x:dbPr", 0);

    assertXPath(pConn, "/x:connections/x:connection/x:olapPr", 0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf167689_x15_namespace)
{
    createScDoc("xlsx/tdf167689_x15_namespace.xlsx");

    save(TestFilter::XLSX);

    xmlDocUniquePtr pConn = parseExport(u"xl/connections.xml"_ustr);
    CPPUNIT_ASSERT(pConn);

    // test if <ext> has xmlns:x15 namespace.
    assertXPathNSDef(pConn, "/x:connections/x:connection[3]/x:extLst/x:ext", "x15",
                     "http://schemas.microsoft.com/office/spreadsheetml/2010/11/main");

    // test id attribute of <x15:connection>
    assertXPath(pConn, "/x:connections/x:connection[3]/x:extLst/x:ext/x15:connection", "id",
                u"Tabelle1");

    // test sourceName attribute of <x15:rangePr>
    assertXPath(pConn, "/x:connections/x:connection[3]/x:extLst/x:ext/x15:connection/x15:rangePr",
                "sourceName", u"_xlcn.LinkedTable_Tabelle1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf166939)
{
    // Given a document with a column autostyle name equal to "a" (it could be any single-character
    // name). Load it as template, to keep streams valid (see ScDocShell::SaveAs) to reuse existing
    // autostyle names (see ScXMLExport::collectAutoStyles).
    loadFromFile(u"ods/autostyle-name-is-single-char.ods",
                 { comphelper::makePropertyValue(u"AsTemplate"_ustr, true) });
    // Saving it must not crash / fail an assertion!
    save(TestFilter::ODS);
    // Check that we tested the codepath preserving existing names - otherwise test makes no sense
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//office:automatic-styles/style:style[@style:name='a']", 1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf166939_1)
{
    // Check that the autostyles are stored correctly, when autostyle names are not standard (are
    // not like "ro1"; the chosen names are "r_1", "r_2"). A mistake had made a function return
    // existing style's index negative, and that wasn't caught in tests...
    loadFromFile(u"fods/lostRowStyle.fods",
                 { comphelper::makePropertyValue(u"AsTemplate"_ustr, true) });
    // Saving it must keep the autostyles
    save(TestFilter::ODS);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(
        pXmlDoc,
        "//office:automatic-styles/style:style[@style:family='table-row'][@style:name='r_1']", 1);
    assertXPath(
        pXmlDoc,
        "//office:automatic-styles/style:style[@style:family='table-row'][@style:name='r_2']", 1);
    assertXPath(pXmlDoc, "//table:table/table:table-row[1]", "style-name", u"r_1");
    // When the bug was introduced, this failed with
    // - In <>, XPath '//table:table/table:table-row[2]' no attribute 'style-name' exist
    assertXPath(pXmlDoc, "//table:table/table:table-row[2]", "style-name", u"r_2");
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf108244)
{
    createScDoc("ods/tdf108244.ods");
    save(TestFilter::FODS);
    xmlDocUniquePtr pXmlDoc = parseExportedFile();
    CPPUNIT_ASSERT(pXmlDoc);

    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getXPathContent(pXmlDoc, "count(//office:annotation)"));
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf150229)
{
    // Create a long URL: longer than Excel allows
    OUString longUrl = []() {
        OUStringBuffer buf("https://www.example.org/query?foo&bar&baz=");
        while (buf.getLength() < 8200)
            buf.append("0123456789");
        return buf.makeStringAndClear();
    }();

    // Create a document, and put that URL as hyperlink into a cell

    createScDoc();

    {
        auto xDoc = mxComponent.queryThrow<sheet::XSpreadsheetDocument>();
        auto xSheets = xDoc->getSheets().queryThrow<container::XIndexAccess>();
        auto xSheet = xSheets->getByIndex(0).queryThrow<sheet::XSpreadsheet>();
        auto xCell = xSheet->getCellByPosition(0, 0).queryThrow<text::XText>();

        auto xFactory = mxComponent.queryThrow<lang::XMultiServiceFactory>();
        auto xField = xFactory->createInstance(u"com.sun.star.text.TextField.URL"_ustr)
                          .queryThrow<beans::XPropertySet>();
        xField->setPropertyValue(u"URL"_ustr, uno::Any(longUrl));
        xField->setPropertyValue(u"Representation"_ustr, uno::Any(u"hyperlink"_ustr));

        xCell->insertTextContent(xCell->getEnd(), xField.queryThrow<text::XTextContent>(), false);
    }

    // Test XLSX export: the hyperlink must truncate at 8192 character boundary

    saveAndReload(TestFilter::XLSX);

    {
        auto xDoc = mxComponent.queryThrow<sheet::XSpreadsheetDocument>();
        auto xSheets = xDoc->getSheets().queryThrow<container::XIndexAccess>();
        auto xSheet = xSheets->getByIndex(0).queryThrow<sheet::XSpreadsheet>();
        auto xCell = xSheet->getCellByPosition(0, 0).queryThrow<text::XText>();
        CPPUNIT_ASSERT_EQUAL(u"hyperlink"_ustr, xCell->getString());
        auto xCellFieldsSupplier = xCell.queryThrow<text::XTextFieldsSupplier>();
        auto xFields = xCellFieldsSupplier->getTextFields()->createEnumeration();
        auto xField = xFields->nextElement().queryThrow<beans::XPropertySet>();
        CPPUNIT_ASSERT_EQUAL(longUrl.copy(0, 8192),
                             xField->getPropertyValue(u"URL"_ustr).get<OUString>());
    }

    // Test the respective OOXML markup

    xmlDocUniquePtr pXml = parseExport(u"xl/worksheets/_rels/sheet1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "//rels:Relationship", "Target", longUrl.subView(0, 8192));

    // Test XLS export: the hyperlink must truncate at 2083 character boundary (that's the limit
    // I found experimentally)

    saveAndReload(TestFilter::XLS);

    {
        auto xDoc = mxComponent.queryThrow<sheet::XSpreadsheetDocument>();
        auto xSheets = xDoc->getSheets().queryThrow<container::XIndexAccess>();
        auto xSheet = xSheets->getByIndex(0).queryThrow<sheet::XSpreadsheet>();
        auto xCell = xSheet->getCellByPosition(0, 0).queryThrow<text::XText>();
        CPPUNIT_ASSERT_EQUAL(u"hyperlink"_ustr, xCell->getString());
        auto xCellFieldsSupplier = xCell.queryThrow<text::XTextFieldsSupplier>();
        auto xFields = xCellFieldsSupplier->getTextFields()->createEnumeration();
        auto xField = xFields->nextElement().queryThrow<beans::XPropertySet>();
        CPPUNIT_ASSERT_EQUAL(longUrl.copy(0, 2083),
                             xField->getPropertyValue(u"URL"_ustr).get<OUString>());
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf165733_leap_day_BCE)
{
    createScDoc();

    {
        ScDocument* pDoc = getScDoc();
        // Set -0001-02-29 (proleptic Gregorian leap day), which is -0001-03-02 in proleptic Julian.
        // If we ever change UI to use proleptic Gregorian, this will change:
        pDoc->SetString(0, 0, 0, u"-0001-03-02"_ustr);
        CPPUNIT_ASSERT_EQUAL(-693900.0, pDoc->GetValue(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(SvNumFormatType::DATE,
                             pDoc->GetFormatTable()->GetType(pDoc->GetNumberFormat(0, 0, 0)));
    }

    saveAndReload(TestFilter::ODS);

    {
        xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "//table:table-cell", "date-value", u"-0001-02-29");

        ScDocument* pDoc = getScDoc();
        // without the fix, this failed with:
        // - Expected: -693900
        // - Actual  : 0
        // because import didn't recognize 1 BCE as a leap year, and considered 29th invalid in Feb:
        CPPUNIT_ASSERT_EQUAL(-693900.0, pDoc->GetValue(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(SvNumFormatType::DATE,
                             pDoc->GetFormatTable()->GetType(pDoc->GetNumberFormat(0, 0, 0)));
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest5, testTdf170565_empty_functions)
{
    createScDoc("ods/tdf170565_empty_functions.ods");

    save(TestFilter::XLSX);
    xmlDocUniquePtr pSheet = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // Without the fix these would've been exported as SUM(), MIN(), MAX(), which aren't valid in Excel
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[1]/x:c[1]/x:f", u"SUM(0)");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[2]/x:c[1]/x:f", u"MIN(#VALUE!)");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[3]/x:c[1]/x:f", u"MAX(#VALUE!)");
    // Further checks to ensure there are no regressions
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[1]/x:c[2]/x:f", u"SUM(,)");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[1]/x:c[3]/x:f", u"SUM(100)");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[1]/x:c[4]/x:f", u"SUM(C1:C3)");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[2]/x:c[2]/x:f", u"MIN(,)");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[2]/x:c[3]/x:f", u"MIN(200)");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[2]/x:c[4]/x:f", u"MIN(C1:C3)");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[3]/x:c[2]/x:f", u"MAX(,)");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[3]/x:c[3]/x:f", u"MAX(-100)");
    assertXPathContent(pSheet, "/x:worksheet/x:sheetData/x:row[3]/x:c[4]/x:f", u"MAX(C1:C3)");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
