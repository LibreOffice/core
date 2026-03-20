/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <comphelper/configuration.hxx>
#include "helper/qahelper.hxx"

#include <global.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <formuladepchain.hxx>
#include <scdll.hxx>

#include <tools/json_writer.hxx>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <functional>
#include <iterator>
#include <sstream>

class ScFormulaDepChainTest : public CppUnit::TestFixture
{
public:
    ScFormulaDepChainTest()
    {
        comphelper::EnableFuzzing();
        ScDLL::Init();
        ScGlobal::Init();
    }

    ~ScFormulaDepChainTest() { ScGlobal::Clear(); }

protected:
    boost::property_tree::ptree runDepChain(ScDocument& rDoc, const ScAddress& rPos)
    {
        tools::JsonWriter aJsonWriter;
        sc::getFormulaDependencyChain(rDoc, rPos, aJsonWriter);
        OString aResult = aJsonWriter.finishAndGetAsOString();

        boost::property_tree::ptree aTree;
        std::stringstream aStream((std::string(aResult)));
        boost::property_tree::read_json(aStream, aTree);
        return aTree;
    }
};

// Non-formula cell should return hasError=false, reason="not_formula"
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testNotFormula)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    aDoc.SetValue(0, 0, 0, 42.0); // A1 = 42

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(false, aTree.get<bool>("hasError"));
    CPPUNIT_ASSERT_EQUAL(std::string("not_formula"), aTree.get<std::string>("reason"));
}

// Formula with no error should return hasError=false, reason="no_error"
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testNoError)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    aDoc.SetString(0, 0, 0, u"=1+1"_ustr); // A1 = =1+1

    // Force recalculation
    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(false, aTree.get<bool>("hasError"));
    CPPUNIT_ASSERT_EQUAL(std::string("no_error"), aTree.get<std::string>("reason"));
}

// Division by zero should produce hasError=true with cell info
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testSimpleError)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    aDoc.SetString(0, 0, 0, u"=1/0"_ustr); // A1 = =1/0

    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    // Check cell node exists with expected fields
    auto aCell = aTree.get_child("cell");
    CPPUNIT_ASSERT(!aCell.get<std::string>("address").empty());
    CPPUNIT_ASSERT(!aCell.get<std::string>("formula").empty());
    CPPUNIT_ASSERT(!aCell.get<std::string>("error").empty());
    CPPUNIT_ASSERT(aCell.get<int>("errorCode") != 0);
}

// Formula referencing a value cell should show that dependency
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testDependencyChain)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    aDoc.SetValue(1, 0, 0, 10.0); // B1 = 10
    aDoc.SetString(0, 0, 0, u"=B1/0"_ustr); // A1 = =B1/0

    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    // Should have dependencies array
    auto aDeps = aTree.get_child("dependencies");
    CPPUNIT_ASSERT(!aDeps.empty());

    // First dependency should be B1 with type "value"
    auto aFirst = aDeps.begin()->second;
    CPPUNIT_ASSERT_EQUAL(std::string("value"), aFirst.get<std::string>("type"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, aFirst.get<double>("value"), 1e-10);
}

// Small range (<=20 cells) should enumerate individual cells
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testRangeDependency)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    aDoc.SetValue(1, 0, 0, 1.0); // B1 = 1
    aDoc.SetValue(1, 1, 0, 2.0); // B2 = 2
    aDoc.SetValue(1, 2, 0, 3.0); // B3 = 3
    aDoc.SetString(0, 0, 0, u"=SUM(B1:B3)/0"_ustr); // A1 = =SUM(B1:B3)/0

    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    auto aDeps = aTree.get_child("dependencies");
    // B1:B3 is 3 cells, should be enumerated individually
    CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(std::distance(aDeps.begin(), aDeps.end())));
}

// Large range (>20 cells) should be summarized
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testLargeRangeSummary)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    // A1 = =SUM(B1:Z100)/0  - range is 25 cols * 100 rows = 2500 cells
    aDoc.SetString(0, 0, 0, u"=SUM(B1:Z100)/0"_ustr);

    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    auto aDeps = aTree.get_child("dependencies");
    CPPUNIT_ASSERT(!aDeps.empty());

    // Should have a single "range" summary entry
    auto aFirst = aDeps.begin()->second;
    CPPUNIT_ASSERT_EQUAL(std::string("range"), aFirst.get<std::string>("type"));
    CPPUNIT_ASSERT(aFirst.get<int>("rows") > 0);
    CPPUNIT_ASSERT(aFirst.get<int>("cols") > 0);
}

// Circular references should be detected
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testCircularReference)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    aDoc.SetString(0, 0, 0, u"=B1"_ustr); // A1 = =B1
    aDoc.SetString(1, 0, 0, u"=A1"_ustr); // B1 = =A1

    // Interpret both
    ScFormulaCell* pCellA = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    ScFormulaCell* pCellB = aDoc.GetFormulaCell(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT(pCellA);
    CPPUNIT_ASSERT(pCellB);
    pCellA->Interpret();
    pCellB->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    // Walk dependencies to find the circular marker
    auto aDeps = aTree.get_child("dependencies");
    bool bFoundCircular = false;
    for (auto& rEntry : aDeps)
    {
        // B1 should be a formula with its own dependencies
        auto optInnerDeps = rEntry.second.get_child_optional("dependencies");
        if (optInnerDeps)
        {
            for (auto& rInner : *optInnerDeps)
            {
                if (rInner.second.get<std::string>("type") == "circular")
                {
                    bFoundCircular = true;
                    break;
                }
            }
        }
    }
    CPPUNIT_ASSERT_MESSAGE("Expected to find a 'circular' type in dependencies", bFoundCircular);
}

// Deep dependency chain should produce "truncated" entries
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testTruncatedOnDepthLimit)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);

    // Create chain: A1=A2, A2=A3, ..., A13=1/0
    // MAX_DEPTH is 10, so at depth 11+ we should see "truncated"
    constexpr int nChainLen = 13;
    for (int i = 0; i < nChainLen - 1; ++i)
    {
        OUString aFormula = "=A" + OUString::number(i + 2);
        aDoc.SetString(0, i, 0, aFormula);
    }
    // Last cell in chain has the actual error
    aDoc.SetString(0, nChainLen - 1, 0, u"=1/0"_ustr);

    // Interpret all cells bottom-up
    for (int i = nChainLen - 1; i >= 0; --i)
    {
        ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, i, 0));
        if (pCell)
            pCell->Interpret();
    }

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    // Walk the dependency chain to find a "truncated" entry
    bool bFoundTruncated = false;
    std::function<void(const boost::property_tree::ptree&)> findTruncated;
    findTruncated = [&](const boost::property_tree::ptree& rNode) {
        auto optType = rNode.get_optional<std::string>("type");
        if (optType && *optType == "truncated")
        {
            bFoundTruncated = true;
            return;
        }
        auto optDeps = rNode.get_child_optional("dependencies");
        if (optDeps)
        {
            for (auto& rEntry : *optDeps)
                findTruncated(rEntry.second);
        }
    };

    auto optDeps = aTree.get_child_optional("dependencies");
    if (optDeps)
    {
        for (auto& rEntry : *optDeps)
            findTruncated(rEntry.second);
    }

    CPPUNIT_ASSERT_MESSAGE("Expected to find a 'truncated' type in deep chain", bFoundTruncated);
}

// Empty cell dependency
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testEmptyDependency)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    // B1 is empty, A1 divides by it (empty = 0 -> div/0)
    aDoc.SetString(0, 0, 0, u"=1/B1"_ustr); // A1 = =1/B1

    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    auto aDeps = aTree.get_child("dependencies");
    CPPUNIT_ASSERT(!aDeps.empty());

    auto aFirst = aDeps.begin()->second;
    CPPUNIT_ASSERT_EQUAL(std::string("empty"), aFirst.get<std::string>("type"));
}

// String dependency
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testStringDependency)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    aDoc.SetString(1, 0, 0, u"hello"_ustr); // B1 = "hello"
    aDoc.SetString(0, 0, 0, u"=B1+1"_ustr); // A1 = =B1+1

    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    // Check if this actually errors - in Calc, "hello"+1 may coerce to 0+1=1
    FormulaError nErr = pCell->GetRawError();
    if (nErr == FormulaError::NONE)
    {
        // If no error, verify it returns no_error
        auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(false, aTree.get<bool>("hasError"));
        return;
    }

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    auto aDeps = aTree.get_child("dependencies");
    CPPUNIT_ASSERT(!aDeps.empty());

    auto aFirst = aDeps.begin()->second;
    CPPUNIT_ASSERT_EQUAL(std::string("string"), aFirst.get<std::string>("type"));
    CPPUNIT_ASSERT_EQUAL(std::string("hello"), aFirst.get<std::string>("value"));
}

// Error formula with no cell references should have no dependencies key
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testNoDepErrorFormula)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    aDoc.SetString(0, 0, 0, u"=1/0"_ustr); // A1 = =1/0 (no cell refs)

    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));
    // No cell references means no dependencies key
    auto optDeps = aTree.get_child_optional("dependencies");
    CPPUNIT_ASSERT_MESSAGE("=1/0 has no cell refs, should have no dependencies", !optDeps);
}

// Formula with multiple dependencies of different types
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testMultipleDependencies)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);
    aDoc.SetValue(0, 0, 0, 10.0); // A1 = 10 (value)
    aDoc.SetString(1, 0, 0, u"hello"_ustr); // B1 = "hello" (string)
    // C1 is empty
    aDoc.SetString(3, 0, 0, u"=(A1+B1+C1)/0"_ustr); // D1 = =(A1+B1+C1)/0

    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(3, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(3, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    auto aDeps = aTree.get_child("dependencies");
    int nCount = 0;
    bool bFoundValue = false, bFoundString = false, bFoundEmpty = false;
    for (auto& rEntry : aDeps)
    {
        std::string sType = rEntry.second.get<std::string>("type");
        if (sType == "value")
            bFoundValue = true;
        else if (sType == "string")
            bFoundString = true;
        else if (sType == "empty")
            bFoundEmpty = true;
        nCount++;
    }
    CPPUNIT_ASSERT_EQUAL(3, nCount);
    CPPUNIT_ASSERT(bFoundValue);
    CPPUNIT_ASSERT(bFoundString);
    CPPUNIT_ASSERT(bFoundEmpty);
}

// Exceeding MAX_CELLS (50) should produce truncated entries
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testCellCountLimit)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);

    // Create 55 value cells in B1..B55
    for (SCROW nRow = 0; nRow < 55; ++nRow)
        aDoc.SetValue(1, nRow, 0, static_cast<double>(nRow + 1));

    // Build a formula referencing all 55 cells: =B1+B2+...+B55 then /0
    OUString aFormula = "=(";
    for (int i = 1; i <= 55; ++i)
    {
        if (i > 1)
            aFormula += "+";
        aFormula += "B" + OUString::number(i);
    }
    aFormula += ")/0";
    aDoc.SetString(0, 0, 0, aFormula); // A1

    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    auto aDeps = aTree.get_child("dependencies");
    // Should have some entries, but not all 55 (MAX_CELLS=50 limits it)
    int nCount = 0;
    bool bFoundTruncated = false;
    for (auto& rEntry : aDeps)
    {
        auto optType = rEntry.second.get_optional<std::string>("type");
        if (optType && *optType == "truncated")
            bFoundTruncated = true;
        nCount++;
    }
    // Should have stopped before emitting all 55
    CPPUNIT_ASSERT(nCount < 55);
    CPPUNIT_ASSERT_MESSAGE("Expected truncated entry when exceeding MAX_CELLS", bFoundTruncated);
}

// Large range with error cells should report errorCells
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testLargeRangeWithErrors)
{
    ScDocument aDoc;
    aDoc.InsertTab(0, u"test"_ustr);

    // Put error formulas inside the range B1:Z100
    aDoc.SetString(1, 0, 0, u"=1/0"_ustr); // B1 = =1/0
    aDoc.SetString(1, 1, 0, u"=1/0"_ustr); // B2 = =1/0

    // Interpret the error cells
    for (SCROW nRow = 0; nRow < 2; ++nRow)
    {
        ScFormulaCell* pErr = aDoc.GetFormulaCell(ScAddress(1, nRow, 0));
        if (pErr)
            pErr->Interpret();
    }

    // A1 references the large range containing those error cells
    aDoc.SetString(0, 0, 0, u"=SUM(B1:Z100)/0"_ustr);

    ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pCell);
    pCell->Interpret();

    auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));

    CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

    auto aDeps = aTree.get_child("dependencies");
    auto aFirst = aDeps.begin()->second;
    CPPUNIT_ASSERT_EQUAL(std::string("range"), aFirst.get<std::string>("type"));

    // Should have errorCells since B1 and B2 have errors
    auto optErrorCells = aFirst.get_optional<std::string>("errorCells");
    CPPUNIT_ASSERT_MESSAGE("Expected errorCells present in range with error formulas",
                           static_cast<bool>(optErrorCells));
    CPPUNIT_ASSERT_MESSAGE("Expected errorCells non-empty in range with error formulas",
                           !optErrorCells->empty());
}

// Range boundary: exactly 20 cells enumerated, 21 cells summarized
CPPUNIT_TEST_FIXTURE(ScFormulaDepChainTest, testRangeBoundary)
{
    // Test 1: 4 cols x 5 rows = 20 cells (exactly MAX_RANGE_CELLS) -> enumerate
    {
        ScDocument aDoc;
        aDoc.InsertTab(0, u"test"_ustr);
        // B1:E5 is 4 cols x 5 rows = 20 cells
        aDoc.SetString(0, 0, 0, u"=SUM(B1:E5)/0"_ustr);

        ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
        CPPUNIT_ASSERT(pCell);
        pCell->Interpret();

        auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

        auto aDeps = aTree.get_child("dependencies");
        int nCount = 0;
        for (auto& rEntry : aDeps)
        {
            // Each should be an individually enumerated cell, not a "range" summary
            CPPUNIT_ASSERT(rEntry.second.get<std::string>("type") != "range");
            nCount++;
        }
        CPPUNIT_ASSERT_EQUAL(20, nCount);
    }

    // Test 2: 3 cols x 7 rows = 21 cells (exceeds MAX_RANGE_CELLS) -> summarize
    {
        ScDocument aDoc;
        aDoc.InsertTab(0, u"test"_ustr);
        // B1:D7 is 3 cols x 7 rows = 21 cells
        aDoc.SetString(0, 0, 0, u"=SUM(B1:D7)/0"_ustr);

        ScFormulaCell* pCell = aDoc.GetFormulaCell(ScAddress(0, 0, 0));
        CPPUNIT_ASSERT(pCell);
        pCell->Interpret();

        auto aTree = runDepChain(aDoc, ScAddress(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(true, aTree.get<bool>("hasError"));

        auto aDeps = aTree.get_child("dependencies");
        // Should be a single range summary, not 21 individual cells
        CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(std::distance(aDeps.begin(), aDeps.end())));
        auto aFirst = aDeps.begin()->second;
        CPPUNIT_ASSERT_EQUAL(std::string("range"), aFirst.get<std::string>("type"));
        CPPUNIT_ASSERT_EQUAL(7, aFirst.get<int>("rows"));
        CPPUNIT_ASSERT_EQUAL(3, aFirst.get<int>("cols"));
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
