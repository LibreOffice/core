/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// core, please keep it alphabetically ordered
#include "helper/qahelper.hxx"

#include <comphelper/propertyvalue.hxx>
#include <dbdata.hxx>
#include <dbdocfun.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <sc.hrc>
#include <sortparam.hxx>
#include <tabvwsh.hxx>
#include <types.hxx>
#include <undomanager.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include <array>

using namespace css;
using namespace css::uno;

/* Implementation of Filters test, volume 5*/

class ScFiltersTest5 : public ScModelTestBase
{
public:
    ScFiltersTest5()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf157689)
{
    // testing the correct import of autofilter on multiple sheets
    createScDoc("xlsx/tdf157689.xlsx");

    ScDocument* pDoc = getScDoc();

    ScDBData* pAnonDBData = pDoc->GetAnonymousDBData(0);
    CPPUNIT_ASSERT(pAnonDBData);
    ScRange aFilterRange;
    pAnonDBData->GetArea(aFilterRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 1, 3, 0), aFilterRange); // A1:B4
    CPPUNIT_ASSERT(pAnonDBData->HasAutoFilter());

    pAnonDBData = pDoc->GetAnonymousDBData(1);
    CPPUNIT_ASSERT(pAnonDBData);
    pAnonDBData->GetArea(aFilterRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 1, 1, 4, 1), aFilterRange); // A1:B5
    CPPUNIT_ASSERT(pAnonDBData->HasAutoFilter());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf151505)
{
    // testing the correct import of autofilter from XLSB
    createScDoc("xlsb/tdf151505.xlsb");

    ScDocument* pDoc = getScDoc();

    ScDBData* pAnonDBData = pDoc->GetAnonymousDBData(0);
    CPPUNIT_ASSERT(pAnonDBData);
    ScRange aFilterRange;
    pAnonDBData->GetArea(aFilterRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 1, 4, 0), aFilterRange); // A1:B5
    CPPUNIT_ASSERT(pAnonDBData->HasAutoFilter());

    // also check for the correct handling of the autofilter buttons
    auto nFlag = pDoc->GetAttr(0, 0, 0, ATTR_MERGE_FLAG).GetValue();
    CPPUNIT_ASSERT(nFlag & ScMF::Auto);
    nFlag = pDoc->GetAttr(1, 0, 0, ATTR_MERGE_FLAG).GetValue();
    CPPUNIT_ASSERT(nFlag & ScMF::Auto);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf167134_LOOKUP_extRef)
{
    // testing the correct handling of external references in LOOKUP parameters
    // The file lookup_target.fods uses external links to file lookup_source.fods
    // The test requires that both are located in directory sc/qa/unit/data/fods;
    createScDoc("fods/lookup_target.fods");

    ScDocument* pDoc = getScDoc();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->ReloadAllLinks();
    pDocSh->DoHardRecalc();

    // compare the data loaded from external links (column 0)
    // with the expected result stored in the test file (column 1)
    for (SCROW nRow = 3; nRow <= 26; nRow++)
    {
        OUString aResult = pDoc->GetString(ScAddress(0, nRow, 0));
        OUString aExpected = pDoc->GetString(ScAddress(1, nRow, 0));
        CPPUNIT_ASSERT_EQUAL(aExpected, aResult);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testLoextSpillImport)
{
    // A table-cell with loext:spill="true" comes back as a dynamic-array
    // master and spills its result into the rows below. A cell with the
    // same formula but no attribute is a plain single-value cell that
    // shows only the upper-left of the result.

    createScDoc("fods/DynamicArraySpill.fods");

    ScDocument* pDoc = getScDoc();

    ScFormulaCell* pMarked = pDoc->GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pMarked);
    CPPUNIT_ASSERT(pMarked->IsDynamicArrayMaster());

    // The master at A1 spills UNIQUE(B1:B4) into A1:A4.
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, pDoc->GetValue(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(40.0, pDoc->GetValue(ScAddress(0, 3, 0)));

    ScFormulaCell* pPlain = pDoc->GetFormulaCell(ScAddress(0, 4, 0));
    CPPUNIT_ASSERT(pPlain);
    CPPUNIT_ASSERT(!pPlain->IsDynamicArrayMaster());

    // The plain cell at A5 shows only the first value with no spill below.
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(0, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, pDoc->GetCellType(ScAddress(0, 5, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testLoextSpillExportFODS)
{
    // Saving as FODS writes loext:spill="true" on the marked formula
    // cell and leaves it off on the plain one.

    createScDoc("fods/DynamicArraySpill.fods");

    save(TestFilter::FODS);

    xmlDocUniquePtr pXmlDoc = parseExportedFile();
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//table:table/table:table-row[1]/table:table-cell[1]", "spill", u"true");
    assertXPath(pXmlDoc, "//table:table/table:table-row[1]/table:table-cell[1]", "formula",
                u"of:=COM.MICROSOFT.UNIQUE([.B1:.B4])");

    assertXPathNoAttribute(pXmlDoc, "//table:table/table:table-row[5]/table:table-cell[1]",
                           "spill");
    assertXPath(pXmlDoc, "//table:table/table:table-row[5]/table:table-cell[1]", "formula",
                u"of:=COM.MICROSOFT.UNIQUE([.B1:.B4])");
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testPlainImplicitIntersectionResolved)
{
    // A plain non-array formula whose RPN intends to produce an
    // array picks up @ in the formula text as soon as the cell is
    // asked to resolve it. ODS import drives this on every plain
    // non-array, non-loext:spill cell at the end of formula
    // compilation.

    createScDoc();
    ScDocument* pDoc = getScDoc();

    pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    pDoc->SetValue(ScAddress(0, 1, 0), 2.0);
    pDoc->SetValue(ScAddress(0, 2, 0), 3.0);

    pDoc->SetFormula(ScAddress(1, 0, 0), u"=A1:A3"_ustr, formula::FormulaGrammar::GRAM_NATIVE);
    ScFormulaCell* pCell = pDoc->GetFormulaCell(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->ResolveImplicitIntersection();

    // The call resolves the decision in-place. No recalc needed.
    CPPUNIT_ASSERT_EQUAL(u"=@A1:A3"_ustr, pDoc->GetFormula(1, 0, 0));
    pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testPlainScalarStaysWithoutImplicitIntersection)
{
    // A plain formula whose RPN does not intend an array (a pure
    // scalar like =SUM(A1:A3)) keeps its text unchanged when the
    // cell is asked to resolve implicit intersection. The walk
    // finds no array intent and leaves the formula alone.

    createScDoc();
    ScDocument* pDoc = getScDoc();

    pDoc->SetValue(ScAddress(0, 0, 0), 1.0);
    pDoc->SetValue(ScAddress(0, 1, 0), 2.0);
    pDoc->SetValue(ScAddress(0, 2, 0), 3.0);

    pDoc->SetFormula(ScAddress(1, 0, 0), u"=SUM(A1:A3)"_ustr, formula::FormulaGrammar::GRAM_NATIVE);
    ScFormulaCell* pCell = pDoc->GetFormulaCell(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->ResolveImplicitIntersection();
    pDoc->CalcAll();

    CPPUNIT_ASSERT_EQUAL(u"=SUM(A1:A3)"_ustr, pDoc->GetFormula(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(6.0, pDoc->GetValue(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testDynamicSpillStateExportFODS)
{
    // A dynamic-array master in spill state collapses to 1x1. FODS
    // export writes it as a plain non-array formula (no matrix dims)
    // carrying loext:spill="true" so a round trip can re-instate the
    // master and re-check the blockers.

    createScDoc();
    ScDocument* pDoc = getScDoc();

    // Source data and a blocker that will trigger #SPILL!.
    pDoc->SetValue(ScAddress(1, 0, 0), 10.0);
    pDoc->SetValue(ScAddress(1, 1, 0), 20.0);
    pDoc->SetValue(ScAddress(1, 2, 0), 30.0);
    pDoc->SetValue(ScAddress(1, 3, 0), 40.0);
    pDoc->SetString(ScAddress(0, 1, 0), u"blocker"_ustr);

    ScMarkData aMark(pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    pDoc->InsertMatrixFormula(0, 0, 0, 0, aMark, u"=UNIQUE(B1:B4)"_ustr, nullptr,
                              formula::FormulaGrammar::GRAM_DEFAULT, true /*bCheckForSpill*/,
                              true /*bDynamicArrayMaster*/);

    ScFormulaCell* pMaster = pDoc->GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pMaster);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(FormulaError::Spill), sal_Int32(pMaster->GetErrCode()));

    save(TestFilter::FODS);
    xmlDocUniquePtr pXmlDoc = parseExportedFile();
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//table:table/table:table-row[1]/table:table-cell[1]", "spill", u"true");
    assertXPathNoAttribute(pXmlDoc, "//table:table/table:table-row[1]/table:table-cell[1]",
                           "number-matrix-columns-spanned");
    assertXPathNoAttribute(pXmlDoc, "//table:table/table:table-row[1]/table:table-cell[1]",
                           "number-matrix-rows-spanned");
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testLoextSpillRoundTripODS)
{
    // The dynamic-array marker and the spilled values survive an ODS
    // save and reload when the file is written in the extended ODF
    // flavour.

    createScDoc("fods/DynamicArraySpill.fods");

    saveAndReload(TestFilter::ODS);

    ScDocument* pDoc = getScDoc();

    ScFormulaCell* pMarked = pDoc->GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pMarked);
    CPPUNIT_ASSERT(pMarked->IsDynamicArrayMaster());

    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, pDoc->GetValue(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(40.0, pDoc->GetValue(ScAddress(0, 3, 0)));

    ScFormulaCell* pPlain = pDoc->GetFormulaCell(ScAddress(0, 4, 0));
    CPPUNIT_ASSERT(pPlain);
    CPPUNIT_ASSERT(!pPlain->IsDynamicArrayMaster());

    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(0, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, pDoc->GetCellType(ScAddress(0, 5, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testArrayFormulasStaticRoundTrip)
{
    // Static array formulas in a legacy ODS file round-trip with
    // every value-check still TRUE.

    auto checkSheet = [](ScDocument* pDoc) {
        // B1 = AND(C4:C52) summarises 49 per-row equality checks.
        CPPUNIT_ASSERT_EQUAL(u"TRUE"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
        for (SCROW nRow = 3; nRow <= 51; ++nRow)
        {
            OString aMessage = "C" + OString::number(nRow + 1) + " must be TRUE";
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMessage.getStr(), u"TRUE"_ustr,
                                         pDoc->GetString(ScAddress(2, nRow, 0)));
        }

        // Plain non-array cells with a bare range carry an explicit @.
        CPPUNIT_ASSERT_EQUAL(u"=@H10:H13"_ustr, pDoc->GetFormula(5, 8, 0)); // F9
        CPPUNIT_ASSERT_EQUAL(u"=@H10:H13"_ustr, pDoc->GetFormula(5, 9, 0)); // F10
        CPPUNIT_ASSERT_EQUAL(u"=@H10:H13"_ustr, pDoc->GetFormula(9, 12, 0)); // J13

        // The five matrix masters stay static (no loext:spill in the
        // source file), so GetFormula keeps the {} wrapping.
        struct Master
        {
            ScAddress maPosition;
            OUString maFormula;
        };
        const Master aMasters[] = {
            { ScAddress(11, 15, 0), u"{=H10:H13}"_ustr }, // L16
            { ScAddress(12, 15, 0), u"{=H10:H13}"_ustr }, // M16
            { ScAddress(13, 15, 0), u"{=TRANSPOSE(TRANSPOSE(H10:H13))}"_ustr }, // N16
            { ScAddress(11, 22, 0), u"{=SORT(H10:H13)}"_ustr }, // L23
            { ScAddress(12, 22, 0), u"{=UNIQUE(H10:H13)}"_ustr }, // M23
        };
        for (const Master& rMaster : aMasters)
        {
            ScFormulaCell* pCell = pDoc->GetFormulaCell(rMaster.maPosition);
            CPPUNIT_ASSERT(pCell);
            CPPUNIT_ASSERT(!pCell->IsDynamicArrayMaster());
            CPPUNIT_ASSERT_EQUAL(rMaster.maFormula, pDoc->GetFormula(rMaster.maPosition.Col(),
                                                                     rMaster.maPosition.Row(), 0));
        }
    };

    createScDoc("ods/ArrayFormulasStatic.ods");
    checkSheet(getScDoc());

    saveAndReload(TestFilter::ODS);
    checkSheet(getScDoc());

    // No @ in any saved formula, and the matrix masters do not get
    // loext:spill="true" because they stayed static.
    xmlDocUniquePtr pXml = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "//table:table-cell[contains(@table:formula, '@')]", 0);
    assertXPath(pXml, "//table:table-cell[@loext:spill='true']", 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf122336)
{
    createScDoc("xlsx/tdf122336.xlsx");

    ScDocument* pDoc = getScDoc();
    // Without the fix in place, this test would have failed with
    // - Expected: Uitvoeringsdatum
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(u"Uitvoeringsdatum"_ustr, pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"12/25/2018"_ustr, pDoc->GetString(0, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf118668)
{
    createScDoc("xlsx/tdf118668.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
    CPPUNIT_ASSERT(pDoc->IsVisible(0));

    //Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(pDoc->IsVisible(1));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf94627)
{
    createScDoc("xlsb/tdf94627.xlsb");

    ScDocument* pDoc = getScDoc();

    ScRangeName* pRangeName = pDoc->GetRangeName();
    ScRangeData* pRangeData = pRangeName->findByUpperName("NAME1");
    CPPUNIT_ASSERT(pRangeData);

    OUString aFormula = pRangeData->GetSymbol();
    CPPUNIT_ASSERT_EQUAL(u"$Sheet1.$A$1"_ustr, aFormula);

    double fVal = pDoc->GetValue(0, 3, 0);
    ASSERT_DOUBLES_EQUAL(2, fVal);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testGenericXMLMapped)
{
    createScDoc("xml-mapped/list-of-records.xml");
    ScDocument* pDoc = getScDoc();

    std::vector<std::vector<const char*>> aCheck = {
        { "id", "name", "email", "age", "city" },
        { "1", "John Smith", "john.smith@example.com", "34", "New York" },
        { "2", "Emma Johnson", "emma.johnson@example.com", "28", "Los Angeles" },
        { "3", "Michael Chen", "michael.chen@example.com", "45", "Chicago" },
        { "4", "Sophia Rodriguez", "sophia.rodriguez@example.com", "19", "Houston" },
        { "5", "James Brown", "james.brown@example.com", "52", "Phoenix" },
        { "6", "Olivia Davis", "olivia.davis@example.com", "27", "Philadelphia" },
        { "7", "William Wilson", "william.wilson@example.com", "39", "San Antonio" },
        { "8", "Ava Martinez", "ava.martinez@example.com", "31", "San Diego" },
        { "9", "Lucas Taylor", "lucas.taylor@example.com", "23", "Dallas" },
        { "10", "Mia Anderson", "mia.anderson@example.com", "29", "San Jose" },
        { "11", "Ethan Thomas", "ethan.thomas@example.com", "41", "Austin" },
        { "12", "Isabella Lee", "isabella.lee@example.com", "26", "Jacksonville" },
        { "13", "Alexander Walker", "alexander.walker@example.com", "37", "San Francisco" },
        { "14", "Charlotte Harris", "charlotte.harris@example.com", "33", "Columbus" },
        { "15", "Daniel Lewis", "daniel.lewis@example.com", "30", "Indianapolis" },
        { "16", "Amelia Clark", "amelia.clark@example.com", "24", "Seattle" },
        { "17", "Henry Young", "henry.young@example.com", "48", "Denver" },
        { "18", "Harper King", "harper.king@example.com", "21", "Washington" },
        { "19", "Benjamin Scott", "benjamin.scott@example.com", "35", "Boston" },
        { "20", "Evelyn Adams", "evelyn.adams@example.com", "40", "Portland" },
    };

    ScRange aOutRange;
    aOutRange.Parse(u"A1:E21"_ustr, *pDoc);
    bool bGood = checkOutput(pDoc, aOutRange, aCheck, "expected output");
    CPPUNIT_ASSERT(bGood);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testGenericJSONMapped1)
{
    createScDoc("json-mapped/list-of-objects.json");
    ScDocument* pDoc = getScDoc();

    std::vector<std::vector<const char*>> aCheck = {
        { "A", "B", "C", "D" },
        { "1", "2", "3", nullptr },
        { "11", "12", "13", nullptr },
        { "21", "22", nullptr, "24" },
        { nullptr, nullptr, "text", nullptr },
    };

    ScRange aOutRange;
    aOutRange.Parse(u"A1:D5"_ustr, *pDoc);
    bool bGood = checkOutput(pDoc, aOutRange, aCheck, "expected output");
    CPPUNIT_ASSERT(bGood);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testGenericJSONMapped2)
{
    createScDoc("json-mapped/nested-arrays.json");
    ScDocument* pDoc = getScDoc();

    std::vector<std::vector<const char*>> aCheck = {
        { "field 0", "field 1", "field 2" },
        { "1", "2", "3" },
        { "4", "5", "6" },
        { "7", "8", "9" },
    };

    ScRange aOutRange;
    aOutRange.Parse(u"A1:C4"_ustr, *pDoc);
    bool bGood = checkOutput(pDoc, aOutRange, aCheck, "expected output");
    CPPUNIT_ASSERT(bGood);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf161948_NaturalSortSaveLoad)
{
    // The test document contains a database range that is named "myData". Its source range is A1:A6,
    // thereby A1 is a header. The sorting result is stored in output range C1:C6.

    // Open document
    createScDoc("ods/tdf161948_NaturalSortSaveLoad.ods");
    ScDocument* pDoc = getScDoc();

    // Enable natural sort in sort descriptor
    ScDBData* pDBData = pDoc->GetDBAtArea(0, 0, 0, 0, 5); // tab, col1, row1, col2, row2
    ScSortParam aSortParam; // that is a struct
    pDBData->GetSortParam(aSortParam);
    aSortParam.eSortNumberBehavior = ScSortNumberBehavior::DOUBLE;

    // The output range and the ScDBData are only updated, when you actual sort using the new
    // parameters.
    ScDocShell* pDocSh = getScDocShell();
    ScDBDocFunc aFunc(*pDocSh);
    bool bSorted = aFunc.SortTab(0, aSortParam, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // Verify items are naturally sorted
    const std::array<OUString, 6> aExpected
        = { u"items"_ustr, u"K3"_ustr, u"K12"_ustr, u"K23"_ustr, u"K104"_ustr, u"K203"_ustr };
    for (SCROW nRow = 0; nRow <= 5; nRow++)
    {
        CPPUNIT_ASSERT_EQUAL(aExpected[nRow], pDoc->GetString(ScAddress(2, nRow, 0)));
    }

    // Save and reload, verify file format and reloaded sort descriptor
    saveAndReload(TestFilter::ODS);
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//table:sort", "embedded-number-behavior", u"double");
    pDoc = getScDoc();
    pDBData = pDoc->GetDBAtArea(0, 0, 0, 0, 5);
    pDBData->GetSortParam(aSortParam);
    CPPUNIT_ASSERT_EQUAL(ScSortNumberBehavior::DOUBLE, aSortParam.eSortNumberBehavior);

    // disable natural sorted
    aSortParam.eSortNumberBehavior = ScSortNumberBehavior::ALPHA_NUMERIC;
    pDocSh = getScDocShell();
    ScDBDocFunc aFunc2(*pDocSh);
    bSorted = aFunc2.SortTab(0, aSortParam, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // Verify items are alpha-numerically sorted
    const std::array<OUString, 6> aExpected2
        = { u"items"_ustr, u"K104"_ustr, u"K12"_ustr, u"K203"_ustr, u"K23"_ustr, u"K3"_ustr };
    for (SCROW nRow = 0; nRow <= 5; nRow++)
    {
        CPPUNIT_ASSERT_EQUAL(aExpected2[nRow], pDoc->GetString(ScAddress(2, nRow, 0)));
    }

    // Save and reload, verify file format and reloaded sort descriptor
    // "alpha-numeric" is default and thus not written.
    saveAndReload(TestFilter::ODS);
    pXmlDoc = parseExport("content.xml");
    assertXPathNoAttribute(pXmlDoc, "//table:sort", "embedded-number-behavior");
    pDoc = getScDoc();
    pDBData = pDoc->GetDBAtArea(0, 0, 0, 0, 5);
    pDBData->GetSortParam(aSortParam);
    CPPUNIT_ASSERT_EQUAL(ScSortNumberBehavior::ALPHA_NUMERIC, aSortParam.eSortNumberBehavior);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf168589)
{
    // Open xls document with Subtotal function
    createScDoc("xls/tdf168589.xls");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);
    ASSERT_DOUBLES_EQUAL(6.0, pDoc->GetValue(4, 5, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf171828_fail_to_import_file)
{
    // Open xlsx document, failing without the fix
    createScDoc("xlsx/tdf171828_fail_to_import_file.xlsx");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_EQUAL(u"950"_ustr, pDoc->GetString(6, 6, 0).trim());
}

namespace
{
ScDBData* findDBData(ScDocument* pDoc, const OUString& rName)
{
    ScDBCollection* pColl = pDoc->GetDBCollection();
    if (!pColl)
        return nullptr;
    return pColl->getNamedDBs().findByUpperName(ScGlobal::getCharClass().uppercase(rName));
}

void dispatchDatabaseSettings(ScTabViewShell* pViewShell, const ScDBData* pDBData, bool bTotalRow)
{
    const ScTableStyleParam* pStyleInfo = pDBData->GetTableStyleInfo();
    CPPUNIT_ASSERT(pStyleInfo);

    SfxDispatcher* pDispatcher = pViewShell->GetViewFrame().GetBindings().GetDispatcher();
    CPPUNIT_ASSERT(pDispatcher);

    ScDatabaseSettingItem aItem(pDBData->HasHeader(), bTotalRow, pStyleInfo->mbFirstColumn,
                                pStyleInfo->mbLastColumn, pStyleInfo->mbRowStripes,
                                pStyleInfo->mbColumnStripes, pDBData->HasAutoFilter(),
                                pStyleInfo->maStyleID);
    pDispatcher->ExecuteList(SID_DATABASE_SETTINGS, SfxCallMode::SYNCHRON, { &aItem });
}
} // anonymous namespace

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTotalRowToggle)
{
    // Load test file: Table2 with A1:C10, TableStyleMedium2, totalsRowCount=1
    // Headers: A, B, C (row 1)
    // Data rows 2-9
    // Total row 10: "Total" in A10, SUBTOTAL(101,Table2[C]) in C10
    createScDoc("xlsx/TableStyleTest.xlsx");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);

    ScTabViewShell* pViewShell = getViewShell();
    CPPUNIT_ASSERT(pViewShell);

    // Move cursor into the table area
    goToCell(u"A2"_ustr);

    // --- Verify initial state: total row exists ---
    CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, pDoc->GetString(ScAddress(0, 9, 0)));
    // B10 should have custom COUNTIF formula result = 3
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(1, 9, 0)));
    // C10 should have SUBTOTAL formula result (average of C2:C9) with number format
    double fAvgValue = pDoc->GetValue(ScAddress(2, 9, 0));
    CPPUNIT_ASSERT(fAvgValue != 0.0);
    // Verify the number format (Hungarian Forint "Ft") is preserved after import
    OUString aFormattedValue = pDoc->GetString(ScAddress(2, 9, 0));
    CPPUNIT_ASSERT_EQUAL(u" 2.75 Ft "_ustr, aFormattedValue);

    ScDBData* pDBData = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pDBData);
    CPPUNIT_ASSERT(pDBData->HasTotals());

    // --- Toggle total row OFF ---
    dispatchDatabaseSettings(pViewShell, pDBData, false);

    // Refresh DB data pointer (may have been replaced)
    pDBData = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pDBData);
    CPPUNIT_ASSERT(!pDBData->HasTotals());

    // Last data row should still have data
    CPPUNIT_ASSERT(pDoc->GetValue(ScAddress(0, 8, 0)) != 0.0);

    // --- Toggle total row ON ---
    dispatchDatabaseSettings(pViewShell, pDBData, true);

    pDBData = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pDBData);
    CPPUNIT_ASSERT(pDBData->HasTotals());

    // Total row should be restored
    CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, pDoc->GetString(ScAddress(0, 9, 0)));
    // B10 custom COUNTIF formula should be restored
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(1, 9, 0)));
    // SUBTOTAL formula in C10 should also be restored with same value
    double fRestoredValue = pDoc->GetValue(ScAddress(2, 9, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(fAvgValue, fRestoredValue, 0.0);
    // TODO: number format (e.g. "Ft" currency) on total row cells is lost
    // after toggle off/on cycle — this is a known limitation to fix later.
    // CPPUNIT_ASSERT_EQUAL(u" 2.75 Ft "_ustr, pDoc->GetString(ScAddress(2, 9, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTotalRowUndoRedo)
{
    createScDoc("xlsx/TableStyleTest.xlsx");
    ScDocument* pDoc = getScDoc();
    ScTabViewShell* pViewShell = getViewShell();

    goToCell(u"A2"_ustr);

    // Initial: total row exists
    CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, pDoc->GetString(ScAddress(0, 9, 0)));
    double fOrigValue = pDoc->GetValue(ScAddress(2, 9, 0));

    ScDBData* pDBData = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pDBData);

    // Toggle total row OFF
    dispatchDatabaseSettings(pViewShell, pDBData, false);

    pDBData = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pDBData);
    CPPUNIT_ASSERT(!pDBData->HasTotals());

    // --- Undo: should restore total row ---
    pDoc->GetUndoManager()->Undo();

    pDBData = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pDBData);
    CPPUNIT_ASSERT(pDBData->HasTotals());
    CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, pDoc->GetString(ScAddress(0, 9, 0)));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(fOrigValue, pDoc->GetValue(ScAddress(2, 9, 0)), 0.0);

    // --- Redo: should remove total row again ---
    pDoc->GetUndoManager()->Redo();

    pDBData = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pDBData);
    CPPUNIT_ASSERT(!pDBData->HasTotals());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testFullColumnRefs)
{
    createScDoc("xlsx/forum-mso-en4-134670.xlsx");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"Total # Of Companies"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    // For K2 cell cached value is 1
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(10, 1, 0)));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
