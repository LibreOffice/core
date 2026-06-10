/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the Collabora Office project.
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
#include <docfunc.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <patattr.hxx>
#include <sc.hrc>
#include <scitems.hxx>
#include <sortparam.hxx>
#include <tabvwsh.hxx>
#include <types.hxx>
#include <undomanager.hxx>

#include <docmodel/color/ComplexColor.hxx>
#include <editeng/brushitem.hxx>

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

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testCoextSpillImport)
{
    // A table-cell with coext:spill="true" comes back as a dynamic-array
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

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testCoextSpillExportFODS)
{
    // Saving as FODS writes coext:spill="true" on the marked formula
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
    // non-array, non-coext:spill cell at the end of formula
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
    // carrying coext:spill="true" so a round trip can re-instate the
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
                              formula::FormulaGrammar::GRAM_DEFAULT, true /*bDynamicArrayMaster*/);

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

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testCoextSpillRoundTripODS)
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

        // The five matrix masters stay static (no coext:spill in the
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
    // coext:spill="true" because they stayed static.
    xmlDocUniquePtr pXml = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    assertXPath(pXml, "//table:table-cell[contains(@table:formula, '@')]", 0);
    assertXPath(pXml, "//table:table-cell[@coext:spill='true']", 0);
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

    ScRangeName& rRangeName = pDoc->GetRangeName();
    ScRangeData* pRangeData = rRangeName.findByUpperName(u"NAME1"_ustr);
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
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
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
    pXmlDoc = parseExport(u"content.xml"_ustr);
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

namespace
{
ScDBData* findDBData(ScDocument* pDoc, const OUString& rName)
{
    ScDBCollection* pColl = pDoc->GetDBCollection();
    if (!pColl)
        return nullptr;
    return pColl->getNamedDBs().findByUpperName(ScGlobal::getCharClass().uppercase(rName));
}

void dispatchDatabaseTotalRow(ScTabViewShell* pViewShell, bool bTotalRow)
{
    SfxDispatcher* pDispatcher = pViewShell->GetViewFrame().GetBindings().GetDispatcher();
    CPPUNIT_ASSERT(pDispatcher);

    SfxBoolItem aItem(SID_TABLE_TOTALROW, bTotalRow);
    pDispatcher->ExecuteList(SID_TABLE_TOTALROW, SfxCallMode::SYNCHRON, { &aItem });
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
    CPPUNIT_ASSERT(pDBData->GetTableStyleInfo());
    CPPUNIT_ASSERT(pDBData->HasTotals());

    // --- Toggle total row OFF ---
    dispatchDatabaseTotalRow(pViewShell, false);

    // Refresh DB data pointer (may have been replaced)
    pDBData = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pDBData);
    CPPUNIT_ASSERT(!pDBData->HasTotals());

    // Last data row should still have data
    CPPUNIT_ASSERT(pDoc->GetValue(ScAddress(0, 8, 0)) != 0.0);

    // --- Toggle total row ON ---
    dispatchDatabaseTotalRow(pViewShell, true);

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
    // Number format (e.g. "Ft" currency) on total row cells is preserved
    // after toggle off/on cycle.
    CPPUNIT_ASSERT_EQUAL(u" 2.75 Ft "_ustr, pDoc->GetString(ScAddress(2, 9, 0)));
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
    CPPUNIT_ASSERT(pDBData->GetTableStyleInfo());

    // Toggle total row OFF
    dispatchDatabaseTotalRow(pViewShell, false);

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

// Two tables stacked, the bottom (Table1) wider than the top (Table2). Toggling Total
// Row ON on the top would, with a plain column-bounded shift, tear Table1 (its D/E
// columns wouldn't move). The fix takes the in-place path: write the total-row cells
// directly into the empty band above the bottom table, no row shift, bottom DBData
// range untouched. Undo wipes the in-place cells and the totals flag; Redo re-applies.
CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTotalRowToggleInPlaceWiderNeighbor)
{
    // Fixture layout:
    //   Table2 (top):    A3:C15  (3 cols), Total Row OFF
    //   empty rows:      A16:E20
    //   Table1 (bottom): A21:E30 (5 cols, straddles top's band), Total Row OFF
    createScDoc("xlsx/toggleTotalTearDet.xlsx");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);

    ScTabViewShell* pViewShell = getViewShell();
    CPPUNIT_ASSERT(pViewShell);

    goToCell(u"A4"_ustr); // inside Table2

    ScDBData* pTop = findDBData(pDoc, u"Table2"_ustr);
    ScDBData* pBottom = findDBData(pDoc, u"Table1"_ustr);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(pBottom);
    CPPUNIT_ASSERT(pTop->GetTableStyleInfo());
    CPPUNIT_ASSERT(pBottom->GetTableStyleInfo());
    CPPUNIT_ASSERT(!pTop->HasTotals());
    CPPUNIT_ASSERT(!pBottom->HasTotals());

    ScRange aBottomBefore;
    pBottom->GetArea(aBottomBefore);

    // --- Toggle Total Row ON on the top table ---
    dispatchDatabaseTotalRow(pViewShell, true);

    pTop = findDBData(pDoc, u"Table2"_ustr);
    pBottom = findDBData(pDoc, u"Table1"_ustr);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(pBottom);
    CPPUNIT_ASSERT(pTop->HasTotals());

    // Top range grew by one row (15 -> 16, i.e. 0-indexed nEndRow 14 -> 15)
    ScRange aTopAfter;
    pTop->GetArea(aTopAfter);
    CPPUNIT_ASSERT_EQUAL(SCROW(15), aTopAfter.aEnd.Row());

    // Bottom range MUST be unchanged (in-place path was taken — no row shift)
    ScRange aBottomAfter;
    pBottom->GetArea(aBottomAfter);
    CPPUNIT_ASSERT_EQUAL(aBottomBefore.aStart.Row(), aBottomAfter.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(aBottomBefore.aEnd.Row(), aBottomAfter.aEnd.Row());

    // Total label written at A16 (0-indexed row 15)
    CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, pDoc->GetString(ScAddress(0, 15, 0)));

    // --- Undo: totals gone, in-place cells wiped, bottom still untouched ---
    pDoc->GetUndoManager()->Undo();

    pTop = findDBData(pDoc, u"Table2"_ustr);
    pBottom = findDBData(pDoc, u"Table1"_ustr);
    CPPUNIT_ASSERT(!pTop->HasTotals());
    pTop->GetArea(aTopAfter);
    CPPUNIT_ASSERT_EQUAL(SCROW(14), aTopAfter.aEnd.Row());
    pBottom->GetArea(aBottomAfter);
    CPPUNIT_ASSERT_EQUAL(aBottomBefore.aStart.Row(), aBottomAfter.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(aBottomBefore.aEnd.Row(), aBottomAfter.aEnd.Row());
    CPPUNIT_ASSERT(!pDoc->HasData(0, 15, 0));

    // --- Redo: totals back, still in-place ---
    pDoc->GetUndoManager()->Redo();

    pTop = findDBData(pDoc, u"Table2"_ustr);
    pBottom = findDBData(pDoc, u"Table1"_ustr);
    CPPUNIT_ASSERT(pTop->HasTotals());
    pTop->GetArea(aTopAfter);
    CPPUNIT_ASSERT_EQUAL(SCROW(15), aTopAfter.aEnd.Row());
    pBottom->GetArea(aBottomAfter);
    CPPUNIT_ASSERT_EQUAL(aBottomBefore.aStart.Row(), aBottomAfter.aStart.Row());
    CPPUNIT_ASSERT_EQUAL(aBottomBefore.aEnd.Row(), aBottomAfter.aEnd.Row());
    CPPUNIT_ASSERT_EQUAL(u"Total"_ustr, pDoc->GetString(ScAddress(0, 15, 0)));
}

// Same wider-neighbour layout, but with content in the in-place destination row before
// the toggle. The operation must be refused (matches Excel) — the existing flag stays
// off, the blocking value stays put, no undo step is recorded.
CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTotalRowToggleRefusedBlockedBand)
{
    createScDoc("xlsx/toggleTotalTearDet.xlsx");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);

    ScTabViewShell* pViewShell = getViewShell();
    CPPUNIT_ASSERT(pViewShell);

    goToCell(u"A4"_ustr);

    // Block the in-place destination row (row 16, 0-indexed 15) in the band
    pDoc->SetValue(ScAddress(1, 15, 0), 99.0);

    ScDBData* pTop = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(pTop->GetTableStyleInfo());
    CPPUNIT_ASSERT(!pTop->HasTotals());

    // Sanity check: the predicate reports refusal up-front (this is the path the
    // sidebar uses to gate the toggle before dispatching).
    CPPUNIT_ASSERT(pTop->WouldTableTotalsBeRefused(true));

    // --- Attempt to toggle Total Row ON — must be refused ---
    dispatchDatabaseTotalRow(pViewShell, true);

    pTop = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(!pTop->HasTotals()); // unchanged
    ScRange aTop;
    pTop->GetArea(aTop);
    CPPUNIT_ASSERT_EQUAL(SCROW(14), aTop.aEnd.Row()); // unchanged

    // Blocking value still where we put it
    CPPUNIT_ASSERT_EQUAL(99.0, pDoc->GetValue(ScAddress(1, 15, 0)));
}

// SID_TABLE_TOTALROW state: when toggling-on is allowed, the slot reports an
// SfxBoolItem matching HasTotals(); when WouldTableTotalsBeRefused is true,
// the slot is DisableItem'd. Reuse the wider-neighbour fixture (Table2 on top
// with Table1 spanning columns D/E below).
CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTotalRowSlotState_Allowed)
{
    createScDoc("xlsx/toggleTotalTearDet.xlsx");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);

    ScTabViewShell* pViewShell = getViewShell();
    CPPUNIT_ASSERT(pViewShell);

    goToCell(u"A4"_ustr); // inside Table2; band below is empty

    ScDBData* pTop = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(!pTop->HasTotals());
    CPPUNIT_ASSERT(!pTop->WouldTableTotalsBeRefused(true));

    std::unique_ptr<SfxPoolItem> pState;
    SfxItemState eState
        = pViewShell->GetViewFrame().GetBindings().QueryState(SID_TABLE_TOTALROW, pState);
    CPPUNIT_ASSERT(eState != SfxItemState::DISABLED);
    const SfxBoolItem* pBoolItem = dynamic_cast<const SfxBoolItem*>(pState.get());
    CPPUNIT_ASSERT(pBoolItem);
    CPPUNIT_ASSERT_EQUAL(false, pBoolItem->GetValue());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTotalRowSlotState_DisabledWhenBlocked)
{
    createScDoc("xlsx/toggleTotalTearDet.xlsx");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);

    ScTabViewShell* pViewShell = getViewShell();
    CPPUNIT_ASSERT(pViewShell);

    // Block the in-place destination row (row 16, 0-indexed 15) in the band
    pDoc->SetValue(ScAddress(1, 15, 0), 99.0);

    goToCell(u"A4"_ustr); // inside Table2

    ScDBData* pTop = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(!pTop->HasTotals());
    CPPUNIT_ASSERT(pTop->WouldTableTotalsBeRefused(true));

    std::unique_ptr<SfxPoolItem> pState;
    SfxItemState eState
        = pViewShell->GetViewFrame().GetBindings().QueryState(SID_TABLE_TOTALROW, pState);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DISABLED, eState);
}

// Recalc-flip suppression: a formula sitting in a styled
// table's adjacency band must NOT trigger auto-expand when it recalcs —
// only direct edits/pastes do. Uses a saved-file fixture so the formula's
// listener subscription is fully established on load.
CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testAutoExpandRecalcFlipDoesNotExpand)
{
    // Fixture: Table2 at A3:C15 (no totals); Table1 at A21:E30 (no totals).
    createScDoc("xlsx/toggleTotalTearDet.xlsx");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc);

    ScDBData* pTop = findDBData(pDoc, u"Table2"_ustr);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(pTop->GetTableStyleInfo());

    ScRange aArea;
    pTop->GetArea(aArea);
    CPPUNIT_ASSERT_EQUAL(SCROW(14), aArea.aEnd.Row());

    // Step 1: type =G33 into A16 (Table2's row band) — the direct-edit
    // broadcast triggers auto-expand. (Formula cell is non-empty
    // regardless of value.)
    insertStringToCell(u"A16"_ustr, u"=G33");
    pTop = findDBData(pDoc, u"Table2"_ustr);
    pTop->GetArea(aArea);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("step 1: user formula expanded Table2", SCROW(15),
                                 aArea.aEnd.Row());

    // Step 2: undo the expansion only. A16 keeps =G33, back inside the band.
    pDoc->GetUndoManager()->Undo();
    pTop = findDBData(pDoc, u"Table2"_ustr);
    pTop->GetArea(aArea);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("step 2: undo restored Table2 area", SCROW(14), aArea.aEnd.Row());

    // Step 3: change G33 → A16 recalcs via FinalTrackFormulas. The band
    // listener catches the recalc-flip and skips. Use docfunc so the drain
    // actually runs via SetDocumentModified.
    ScDocShell* pDocShell = getScDocShell();
    pDocShell->GetDocFunc().SetValueCell(ScAddress(6, 32, 0), 99.0, false);

    // Sanity: prove the formula actually recalc'd (else test is a no-op).
    CPPUNIT_ASSERT_EQUAL_MESSAGE("step 3: formula at A16 must have recalc'd", 99.0,
                                 pDoc->GetValue(ScAddress(0, 15, 0)));

    pTop = findDBData(pDoc, u"Table2"_ustr);
    pTop->GetArea(aArea);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("step 3: recalc-flip must NOT expand Table2", SCROW(14),
                                 aArea.aEnd.Row());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testFullColumnRefs)
{
    createScDoc("xlsx/forum-mso-en4-134670.xlsx");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"Total # Of Companies"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    // For K2 cell cached value is 1
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(10, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testAutoFontColorTransparentInheritedBg)
{
    // A cell whose style sets fo:background-color="transparent" while the
    // parent style supplies a non-transparent color (here "Default" with
    // #ff3333) ends up with a brush that has alpha=0 but inherits the parent
    // RGB. The auto-font-color path used to compare against COL_TRANSPARENT
    // by full mValue equality and miss this case, then call IsDark() on the
    // inherited red and return COL_WHITE - white text on a still-white-painted
    // cell.
    createScDoc("fods/auto-font-color-transparent-cell.fods");

    ScDocument* pDoc = getScDoc();
    const ScPatternAttr* pPattern = pDoc->GetPattern(0, 0, 0);
    CPPUNIT_ASSERT(pPattern);

    // Sanity-check the brush we built up: alpha=0 (fully transparent) but
    // RGB inherited from "Default".
    const SvxBrushItem& rBrush = pPattern->GetItem(ATTR_BACKGROUND);
    CPPUNIT_ASSERT(rBrush.GetColor().IsFullyTransparent());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0xff), rBrush.GetColor().GetRed());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x33), rBrush.GetColor().GetGreen());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(0x33), rBrush.GetColor().GetBlue());

    // Print mode forces the transparent-bg fallback to COL_WHITE, so the
    // resolved auto font color must be COL_BLACK. Without the fix it was
    // COL_WHITE because the inherited red was treated as the cell bg.
    model::ComplexColor aComplexColor;
    pPattern->fillColor(aComplexColor, ScAutoFontColorMode::Print);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, aComplexColor.getFinalColor());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
