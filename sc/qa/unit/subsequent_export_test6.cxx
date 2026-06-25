/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <dbdata.hxx>
#include <docsh.hxx>
#include <formulacell.hxx>
#include <tablestyle.hxx>
#include <attrib.hxx>
#include <validat.hxx>
#include <document.hxx>
#include <typedstrdata.hxx>
#include <rangenam.hxx>
#include <token.hxx>
#include <tokenarray.hxx>
#include <patattr.hxx>

#include <svtools/sfxecode.hxx>
#include <svl/intitem.hxx>
#include <sfx2/docfile.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;

class ScExportTest6 : public ScModelTestBase
{
public:
    ScExportTest6()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(ScExportTest6, testXlStartupExternalXLS)
{
    createScDoc("xls/XlStartupExternal.xls");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pExternalRel
        = parseExport(u"xl/externalLinks/_rels/externalLink1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pExternalRel);

    assertXPath(
        pExternalRel, "/rels:Relationships/rels:Relationship", "Type",
        u"http://schemas.microsoft.com/office/2006/relationships/xlExternalLinkPath/xlStartup");
    assertXPath(pExternalRel, "/rels:Relationships/rels:Relationship", "Target", u"personal.xls");
}

CPPUNIT_TEST_FIXTURE(ScExportTest6, testXlStartupExternalXLSX)
{
    createScDoc("xlsx/XlStartupExternal.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pExternalRel
        = parseExport(u"xl/externalLinks/_rels/externalLink1.xml.rels"_ustr);
    CPPUNIT_ASSERT(pExternalRel);

    assertXPath(
        pExternalRel, "/rels:Relationships/rels:Relationship", "Type",
        u"http://schemas.microsoft.com/office/2006/relationships/xlExternalLinkPath/xlStartup");
    assertXPath(pExternalRel, "/rels:Relationships/rels:Relationship", "Target", u"personal.xls");
}

CPPUNIT_TEST_FIXTURE(ScExportTest6, testShapeMacroExtRef)
{
    createScDoc("xlsx/shape-macro-ext-ref.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pDrawing = parseExport(u"xl/drawings/drawing1.xml"_ustr);
    CPPUNIT_ASSERT(pDrawing);
    assertXPath(pDrawing, "//xdr:sp", "macro", u"[1]!Importieren");

    xmlDocUniquePtr pExtLink = parseExport(u"xl/externalLinks/externalLink1.xml"_ustr);
    CPPUNIT_ASSERT(pExtLink);
    assertXPath(pExtLink, "/x:externalLink/x:externalBook/x:definedNames/x:definedName", "name",
                u"Importieren");
}

// --- Table Style OOXML Export Tests ---

CPPUNIT_TEST_FIXTURE(ScExportTest6, testTableStyleDefaultExportXLSX)
{
    // Open xlsx with default table style (TableStyleMedium2), save as xlsx, verify roundtrip
    createScDoc("xlsx/TableStyleTest.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pTable = parseExport(u"xl/tables/table1.xml"_ustr);
    CPPUNIT_ASSERT(pTable);

    // Table style name should survive roundtrip
    assertXPath(pTable, "/x:table/x:tableStyleInfo", "name", u"TableStyleMedium2");
    assertXPath(pTable, "/x:table/x:tableStyleInfo", "showRowStripes", u"1");
    assertXPath(pTable, "/x:table/x:tableStyleInfo", "showColumnStripes", u"0");
    assertXPath(pTable, "/x:table/x:tableStyleInfo", "showFirstColumn", u"0");
    assertXPath(pTable, "/x:table/x:tableStyleInfo", "showLastColumn", u"0");

    // Verify table range includes total row
    assertXPath(pTable, "/x:table", "ref", u"A1:C10");
    assertXPath(pTable, "/x:table", "totalsRowCount", u"1");
    // totalsRowShown is unnecessary and should not be exported
    assertXPathNoAttribute(pTable, "/x:table", "totalsRowShown");

    // Verify column definitions
    assertXPath(pTable, "/x:table/x:tableColumns/x:tableColumn[1]", "name", u"A");
    assertXPath(pTable, "/x:table/x:tableColumns/x:tableColumn[3]", "name", u"C");

    // Total row: Column A should have "Total" label
    assertXPath(pTable, "/x:table/x:tableColumns/x:tableColumn[1]", "totalsRowLabel", u"Total");

    // Total row: Column B should have custom function (COUNTIF)
    assertXPath(pTable, "/x:table/x:tableColumns/x:tableColumn[2]", "totalsRowFunction", u"custom");
    assertXPathContent(pTable, "/x:table/x:tableColumns/x:tableColumn[2]/x:totalsRowFormula",
                       u"COUNTIF(Table2[B],\">5\")");

    // Total row: Column C should have average function
    assertXPath(pTable, "/x:table/x:tableColumns/x:tableColumn[3]", "totalsRowFunction",
                u"average");
}

CPPUNIT_TEST_FIXTURE(ScExportTest6, testTableStyleCustomExportXLSX)
{
    // Open xlsx with custom table style, save as xlsx, verify roundtrip
    createScDoc("xlsx/Book1_custom.xlsx");
    save(TestFilter::XLSX);

    // Verify table style info
    xmlDocUniquePtr pTable = parseExport(u"xl/tables/table1.xml"_ustr);
    CPPUNIT_ASSERT(pTable);
    assertXPath(pTable, "/x:table/x:tableStyleInfo", "name", u"Custom_Styles1");

    // Verify custom style is exported in styles.xml
    xmlDocUniquePtr pStyles = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pStyles);

    // Custom table style should exist with correct name and 7 elements
    assertXPath(pStyles, "/x:styleSheet/x:tableStyles/x:tableStyle", "name", u"Custom_Styles1");
    assertXPath(pStyles, "/x:styleSheet/x:tableStyles/x:tableStyle", "count", u"7");

    // Verify all 7 table style elements are exported
    const char* const sStylePath = "/x:styleSheet/x:tableStyles/x:tableStyle";
    assertXPath(pStyles, OString::Concat(sStylePath) + "/x:tableStyleElement[@type='wholeTable']");
    assertXPath(pStyles, OString::Concat(sStylePath) + "/x:tableStyleElement[@type='headerRow']");
    assertXPath(pStyles, OString::Concat(sStylePath) + "/x:tableStyleElement[@type='totalRow']");
    assertXPath(pStyles, OString::Concat(sStylePath) + "/x:tableStyleElement[@type='firstColumn']");
    assertXPath(pStyles, OString::Concat(sStylePath) + "/x:tableStyleElement[@type='lastColumn']");
    assertXPath(pStyles,
                OString::Concat(sStylePath) + "/x:tableStyleElement[@type='firstRowStripe']");
    assertXPath(pStyles,
                OString::Concat(sStylePath) + "/x:tableStyleElement[@type='firstColumnStripe']");

    // Verify DXFs exist for the custom style elements
    OUString sDxfCount = getXPath(pStyles, "/x:styleSheet/x:dxfs", "count");
    CPPUNIT_ASSERT(sDxfCount.toInt32() >= 7);

    // Verify each table style element has a valid dxfId attribute
    // (DXF order may change during roundtrip, so we check by element type)
    OString sWholeTable = OString::Concat(sStylePath) + "/x:tableStyleElement[@type='wholeTable']";
    OString sHeaderRow = OString::Concat(sStylePath) + "/x:tableStyleElement[@type='headerRow']";
    OString sTotalRow = OString::Concat(sStylePath) + "/x:tableStyleElement[@type='totalRow']";

    // Each element should have a dxfId pointing to a valid DXF
    sal_Int32 nWholeTableDxfId = getXPath(pStyles, sWholeTable, "dxfId").toInt32();
    sal_Int32 nHeaderRowDxfId = getXPath(pStyles, sHeaderRow, "dxfId").toInt32();
    sal_Int32 nTotalRowDxfId = getXPath(pStyles, sTotalRow, "dxfId").toInt32();

    CPPUNIT_ASSERT(nWholeTableDxfId >= 0);
    CPPUNIT_ASSERT(nHeaderRowDxfId >= 0);
    CPPUNIT_ASSERT(nTotalRowDxfId >= 0);

    // All dxfIds should be different (each element has its own DXF)
    CPPUNIT_ASSERT(nWholeTableDxfId != nHeaderRowDxfId);
    CPPUNIT_ASSERT(nWholeTableDxfId != nTotalRowDxfId);
    CPPUNIT_ASSERT(nHeaderRowDxfId != nTotalRowDxfId);

    // Verify totalRow DXF has double top border (use 1-based index: dxfId + 1)
    OString sTotalRowDxf
        = "/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nTotalRowDxfId + 1) + "]";
    assertXPath(pStyles, sTotalRowDxf + "/x:border/x:top", "style", u"double");

    // Verify wholeTable DXF has thin borders
    OString sWholeTableDxf
        = "/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nWholeTableDxfId + 1) + "]";
    assertXPath(pStyles, sWholeTableDxf + "/x:border/x:left", "style", u"thin");
    assertXPath(pStyles, sWholeTableDxf + "/x:border/x:top", "style", u"thin");

    // Verify firstRowStripe DXF has a fill with theme color
    OString sFirstRowStripe
        = OString::Concat(sStylePath) + "/x:tableStyleElement[@type='firstRowStripe']";
    sal_Int32 nFirstRowStripeDxfId = getXPath(pStyles, sFirstRowStripe, "dxfId").toInt32();
    CPPUNIT_ASSERT(nFirstRowStripeDxfId >= 0);
    OString sFirstRowStripeDxf
        = "/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nFirstRowStripeDxfId + 1) + "]";
    assertXPath(pStyles, sFirstRowStripeDxf + "/x:fill/x:patternFill/x:bgColor", "theme", u"9");
}

CPPUNIT_TEST_FIXTURE(ScExportTest6, testTableStyleInnerBordersExportXLSX)
{
    createScDoc("xlsx/tableStyleInnerBorders.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pStyles = parseExport(u"xl/styles.xml"_ustr);
    CPPUNIT_ASSERT(pStyles);

    assertXPath(pStyles, "/x:styleSheet/x:tableStyles/x:tableStyle", "name", u"Customtablestyle");

    // Locate the wholeTable element's DXF (the DXF order may change on roundtrip).
    sal_Int32 nWholeTableDxfId
        = getXPath(
              pStyles,
              "/x:styleSheet/x:tableStyles/x:tableStyle/x:tableStyleElement[@type='wholeTable']",
              "dxfId")
              .toInt32();
    CPPUNIT_ASSERT(nWholeTableDxfId >= 0);

    OString sBorder
        = "/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nWholeTableDxfId + 1) + "]/x:border";

    // The outer borders already exported correctly before the fix.
    assertXPath(pStyles, sBorder + "/x:left", "style", u"thin");
    assertXPath(pStyles, sBorder + "/x:top", "style", u"thin");

    // The inner vertical/horizontal borders must now be exported too (with their
    // theme color), instead of being silently lost.
    assertXPath(pStyles, sBorder + "/x:vertical", "style", u"thin");
    assertXPath(pStyles, sBorder + "/x:vertical/x:color", "theme", u"8");
    assertXPath(pStyles, sBorder + "/x:horizontal", "style", u"thin");
    assertXPath(pStyles, sBorder + "/x:horizontal/x:color", "theme", u"8");

    sal_Int32 nHeaderRowDxfId
        = getXPath(
              pStyles,
              "/x:styleSheet/x:tableStyles/x:tableStyle/x:tableStyleElement[@type='headerRow']",
              "dxfId")
              .toInt32();
    CPPUNIT_ASSERT(nHeaderRowDxfId >= 0);
    OString sHdrBorder
        = "/x:styleSheet/x:dxfs/x:dxf[" + OString::number(nHeaderRowDxfId + 1) + "]/x:border";
    assertXPath(pStyles, sHdrBorder + "/x:bottom", "style", u"medium");
    assertXPath(pStyles, sHdrBorder + "/x:left", 0);
    assertXPath(pStyles, sHdrBorder + "/x:right", 0);
    assertXPath(pStyles, sHdrBorder + "/x:top", 0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest6, testTableStyleCustomRoundtripXLSX)
{
    // Open xlsx with custom table style, save and reload, verify in-memory state
    createScDoc("xlsx/Book1_custom.xlsx");
    saveAndReload(TestFilter::XLSX);

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);

    // Verify custom table style exists in the document
    ScTableStyles* pStyles = pDoc->GetTableStyles();
    CPPUNIT_ASSERT(pStyles);
    CPPUNIT_ASSERT(pStyles->GetTableStyle(u"Custom_Styles1"_ustr) != nullptr);

    // Verify DB range has the custom style applied
    ScDBData* pDBData = pDoc->GetDBAtArea(0, 0, 0, 3, 10);
    CPPUNIT_ASSERT(pDBData);
    const ScTableStyleParam* pParam = pDBData->GetTableStyleInfo();
    CPPUNIT_ASSERT(pParam);
    CPPUNIT_ASSERT_EQUAL(u"Custom_Styles1"_ustr, pParam->maStyleID);

    // Verify table style options survived roundtrip
    CPPUNIT_ASSERT(pParam->mbRowStripes);
    CPPUNIT_ASSERT(!pParam->mbColumnStripes);
    CPPUNIT_ASSERT(!pParam->mbFirstColumn);
    CPPUNIT_ASSERT(!pParam->mbLastColumn);

    // Verify header and total row flags
    CPPUNIT_ASSERT(pDBData->HasHeader());
    CPPUNIT_ASSERT(pDBData->HasTotals());

    // Verify data survived roundtrip
    CPPUNIT_ASSERT_EQUAL(u"Names"_ustr, pDoc->GetString(ScAddress(0, 0, 0))); // header
    CPPUNIT_ASSERT_EQUAL(u"Summary"_ustr, pDoc->GetString(ScAddress(0, 10, 0))); // total label
    CPPUNIT_ASSERT_EQUAL(382.0, pDoc->GetValue(ScAddress(3, 10, 0))); // SUM of Age column
}

CPPUNIT_TEST_FIXTURE(ScExportTest6, testValidationListNameThroughTableRef)
{
    // A list validation pointing at a name whose body is a table reference used to
    // come up empty; the dropdown must list the referenced table column instead.

    createScDoc();
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);

    pDoc->SetString(ScAddress(1, 1, 0), u"Apple"_ustr);
    pDoc->SetString(ScAddress(1, 2, 0), u"Banana"_ustr);
    pDoc->SetString(ScAddress(1, 3, 0), u"Cherry"_ustr);

    std::unique_ptr<ScDBData> pDB(new ScDBData(u"MyTable"_ustr, 0, 1, 1, 1, 3, true, false));
    ScDBData* pDBRaw = pDB.get();
    CPPUNIT_ASSERT(pDoc->GetDBCollection()->getNamedDBs().insert(std::move(pDB)));

    ScTokenArray aNameTokens(*pDoc);
    aNameTokens.AddTableRef(pDBRaw->GetIndex());
    auto pName
        = std::make_unique<ScRangeData>(*pDoc, u"MyName"_ustr, aNameTokens, ScAddress(0, 0, 0));
    ScRangeData* pNameRaw = pName.get();
    CPPUNIT_ASSERT(pDoc->GetRangeName().insert(std::move(pName)));

    ScTokenArray aValidationTokens(*pDoc);
    aValidationTokens.AddRangeName(pNameRaw->GetIndex(), -1);

    ScValidationData aValidationData(SC_VALID_LIST, ScConditionMode::Equal, &aValidationTokens,
                                     nullptr, *pDoc, ScAddress(0, 0, 0));
    const sal_uInt32 nValidationKey = pDoc->AddValidationEntry(aValidationData);
    CPPUNIT_ASSERT(nValidationKey != 0);

    ScPatternAttr aPattern(pDoc->getCellAttributeHelper());
    aPattern.GetItemSetWritable().Put(SfxUInt32Item(ATTR_VALIDDATA, nValidationKey));
    pDoc->ApplyPattern(3, 3, 0, aPattern);

    const ScValidationData* pData = pDoc->GetValidationEntry(nValidationKey);
    CPPUNIT_ASSERT(pData);
    std::vector<ScTypedStrData> aStrings;
    CPPUNIT_ASSERT(pData->FillSelectionList(aStrings, ScAddress(3, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(size_t(3), aStrings.size());
}

CPPUNIT_TEST_FIXTURE(ScExportTest6, testValidationListNameThroughTableRefWithHeader)
{
    // A list validation pointing at a name that wraps a table reference must show
    // the table's data rows, even when the table has a header row and holds
    // numbers: the dropdown lists the data values in order, starting at the first.

    createScDoc();
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);

    pDoc->SetString(ScAddress(1, 0, 0), u"Score"_ustr);
    pDoc->SetValue(ScAddress(1, 1, 0), 10.0);
    pDoc->SetValue(ScAddress(1, 2, 0), 20.0);
    pDoc->SetValue(ScAddress(1, 3, 0), 30.0);

    // Header row plus three data rows; the data part the reference resolves to
    // starts one row below the header.
    std::unique_ptr<ScDBData> pDB(new ScDBData(u"MyTable"_ustr, 0, 1, 0, 1, 3, true, true));
    ScDBData* pDBRaw = pDB.get();
    CPPUNIT_ASSERT(pDoc->GetDBCollection()->getNamedDBs().insert(std::move(pDB)));

    ScTokenArray aNameTokens(*pDoc);
    aNameTokens.AddTableRef(pDBRaw->GetIndex());
    auto pName
        = std::make_unique<ScRangeData>(*pDoc, u"MyName"_ustr, aNameTokens, ScAddress(0, 0, 0));
    ScRangeData* pNameRaw = pName.get();
    CPPUNIT_ASSERT(pDoc->GetRangeName().insert(std::move(pName)));

    ScTokenArray aValidationTokens(*pDoc);
    aValidationTokens.AddRangeName(pNameRaw->GetIndex(), -1);

    ScValidationData aValidationData(SC_VALID_LIST, ScConditionMode::Equal, &aValidationTokens,
                                     nullptr, *pDoc, ScAddress(0, 0, 0));
    const sal_uInt32 nValidationKey = pDoc->AddValidationEntry(aValidationData);
    CPPUNIT_ASSERT(nValidationKey != 0);

    ScPatternAttr aPattern(pDoc->getCellAttributeHelper());
    aPattern.GetItemSetWritable().Put(SfxUInt32Item(ATTR_VALIDDATA, nValidationKey));
    pDoc->ApplyPattern(3, 3, 0, aPattern);

    const ScValidationData* pData = pDoc->GetValidationEntry(nValidationKey);
    CPPUNIT_ASSERT(pData);
    std::vector<ScTypedStrData> aStrings;
    CPPUNIT_ASSERT(pData->FillSelectionList(aStrings, ScAddress(3, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(size_t(3), aStrings.size());
    CPPUNIT_ASSERT_EQUAL(u"10"_ustr, aStrings[0].GetString());
    CPPUNIT_ASSERT_EQUAL(u"20"_ustr, aStrings[1].GetString());
    CPPUNIT_ASSERT_EQUAL(u"30"_ustr, aStrings[2].GetString());
}

CPPUNIT_TEST_FIXTURE(ScExportTest6, testValidationListNameThroughTableRefColumn)
{
    // A name whose body is a column-qualified table reference like MyTable[Score]
    // must list only that column's data rows, not the first column and not the
    // header. Such a reference compiles to several tokens, which used to leave the
    // dropdown empty.

    createScDoc();
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);

    pDoc->SetString(ScAddress(0, 0, 0), u"Name"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"Score"_ustr);
    pDoc->SetString(ScAddress(0, 1, 0), u"x"_ustr);
    pDoc->SetValue(ScAddress(1, 1, 0), 10.0);
    pDoc->SetString(ScAddress(0, 2, 0), u"y"_ustr);
    pDoc->SetValue(ScAddress(1, 2, 0), 20.0);
    pDoc->SetString(ScAddress(0, 3, 0), u"z"_ustr);
    pDoc->SetValue(ScAddress(1, 3, 0), 30.0);

    // Two-column table A1:B4 with a header row.
    std::unique_ptr<ScDBData> pDB(new ScDBData(u"MyTable"_ustr, 0, 0, 0, 1, 3, true, true));
    CPPUNIT_ASSERT(pDoc->GetDBCollection()->getNamedDBs().insert(std::move(pDB)));

    // Base the name outside the table so the column resolves to the data rows,
    // not an intersecting single cell.
    auto pName = std::make_unique<ScRangeData>(*pDoc, u"MyName"_ustr, u"MyTable[[Score]]"_ustr,
                                               ScAddress(0, 5, 0), ScRangeData::Type::Name,
                                               formula::FormulaGrammar::GRAM_NATIVE);
    // A column-qualified reference is more than one token.
    CPPUNIT_ASSERT(pName->GetCode()->GetLen() > 1);
    ScRangeData* pNameRaw = pName.get();
    CPPUNIT_ASSERT(pDoc->GetRangeName().insert(std::move(pName)));

    ScTokenArray aValidationTokens(*pDoc);
    aValidationTokens.AddRangeName(pNameRaw->GetIndex(), -1);

    ScValidationData aValidationData(SC_VALID_LIST, ScConditionMode::Equal, &aValidationTokens,
                                     nullptr, *pDoc, ScAddress(0, 0, 0));
    const sal_uInt32 nValidationKey = pDoc->AddValidationEntry(aValidationData);
    CPPUNIT_ASSERT(nValidationKey != 0);

    ScPatternAttr aPattern(pDoc->getCellAttributeHelper());
    aPattern.GetItemSetWritable().Put(SfxUInt32Item(ATTR_VALIDDATA, nValidationKey));
    pDoc->ApplyPattern(3, 3, 0, aPattern);

    const ScValidationData* pData = pDoc->GetValidationEntry(nValidationKey);
    CPPUNIT_ASSERT(pData);
    std::vector<ScTypedStrData> aStrings;
    CPPUNIT_ASSERT(pData->FillSelectionList(aStrings, ScAddress(3, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(size_t(3), aStrings.size());
    CPPUNIT_ASSERT_EQUAL(u"10"_ustr, aStrings[0].GetString());
    CPPUNIT_ASSERT_EQUAL(u"20"_ustr, aStrings[1].GetString());
    CPPUNIT_ASSERT_EQUAL(u"30"_ustr, aStrings[2].GetString());
}

CPPUNIT_TEST_FIXTURE(ScExportTest6, testPivotTablesWriteRowColumnItems)
{
    createScDoc("xlsx/forum-mso-de-104083.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pTable = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pTable);

    OString sPath1 = "/x:pivotTableDefinition/x:colItems/x:i[4]"_ostr;

    /*
        earlier
        <i t="default">
            <x v="-1"/>
            <x v="1"/>
        </i>

        now
        <i t="default" r="1">
            <x v="1"/>
        </i>
     */
    assertXPath(pTable, sPath1, "t", u"default");
    assertXPath(pTable, sPath1, "r", u"1");
    assertXPathChildren(pTable, sPath1, 1);
    assertXPath(pTable, sPath1 + "/x:x", "v", u"1");

    /*
        earlier
        <i r="1"> // r = 1 means repeat the years and use the item at index 9 of quarters field, but quarters has only 6 items
            <x v="9"/>
        </i>

        now
        <i r="2">
            <x v="9"/>
        </i>
     */
    assertXPath(pTable, "/x:pivotTableDefinition/x:colItems/x:i[3]", "r", u"2");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
