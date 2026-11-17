/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sctiledrenderingtest.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <COKit/COKit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/kit/helper.hxx>
#include <tools/json_writer.hxx>
#include <vcl/scheduler.hxx>

#include <cellmergeoption.hxx>
#include <dbdocfun.hxx>
#include <docfunc.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <docuno.hxx>
#include <global.hxx>
#include <sctestviewcallback.hxx>
#include <SparklineGroup.hxx>
#include <tabprotection.hxx>
#include <tabvwsh.hxx>

namespace
{
/**
 * Extract from an array of cell ranges a list of strings where each
 * string is formatted as "startCol, startRow, endCol, endRow".
 */
std::vector<std::string> getCellRanges(const boost::property_tree::ptree& rRanges)
{
    std::vector<std::string> aRanges;
    for (const auto& rRange : rRanges)
        aRanges.push_back(rRange.second.get_value<std::string>());
    return aRanges;
}

/**
 * Extract the numbers of a JSON array.
 */
std::vector<sal_Int32> toIntegers(const boost::property_tree::ptree& rArray)
{
    std::vector<sal_Int32> aIndexes;
    for (const auto& rIndex : rArray)
        aIndexes.push_back(rIndex.second.get_value<sal_Int32>());
    return aIndexes;
}

/**
 * Read a range sent as [startCol, startRow, endCol, endRow] into a range on
 * sheet 0.
 */
ScRange toRange(const boost::property_tree::ptree& rArray)
{
    std::vector<sal_Int32> aIndexes = toIntegers(rArray);
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), aIndexes.size());
    return ScRange(aIndexes[0], aIndexes[1], 0, aIndexes[2], aIndexes[3], 0);
}

/**
 * Read an array of ranges.
 */
std::vector<ScRange> toRanges(const boost::property_tree::ptree& rRanges)
{
    std::vector<ScRange> aRanges;
    for (const auto& rRange : rRanges)
        aRanges.push_back(toRange(rRange.second));
    return aRanges;
}

/**
 * Check that a list of ranges is the expected one.
 */
void assertRangesEqual(const std::vector<ScRange>& rExpected, const std::vector<ScRange>& rActual)
{
    CPPUNIT_ASSERT_EQUAL(rExpected.size(), rActual.size());
    for (std::size_t i = 0; i < rExpected.size(); ++i)
        CPPUNIT_ASSERT_EQUAL(rExpected[i], rActual[i]);
}

/**
 * Check that a list of sparkline groups, each a list of ranges, is the
 * expected one.
 */
void assertSparklineGroupsEqual(const std::vector<std::vector<ScRange>>& rExpected,
                                const std::vector<std::vector<ScRange>>& rActual)
{
    CPPUNIT_ASSERT_EQUAL(rExpected.size(), rActual.size());
    for (std::size_t i = 0; i < rExpected.size(); ++i)
        assertRangesEqual(rExpected[i], rActual[i]);
}

/**
 * Get the response to a command as a ptree.
 */
boost::property_tree::ptree getCommandResponse(ScModelObj& rModelObj, std::string_view aCommand)
{
    tools::JsonWriter aWriter;
    rModelObj.getCommandValues(aWriter, aCommand);
    std::stringstream aStream(std::string(aWriter.finishAndGetAsOString()));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    return aTree;
}

/**
 * Get the sparkline groups of sheet 0 from the cell range inventory,
 * with each group sorted to make the result deterministic.
 */
std::vector<std::vector<ScRange>> getSparklineGroups(ScModelObj& rModelObj)
{
    boost::property_tree::ptree aTree
        = getCommandResponse(rModelObj, ".uno:CellRangeInventory?part=0");
    std::vector<std::vector<ScRange>> aGroups;
    for (const auto& rGroup : aTree.get_child("commandValues.sparklineGroups"))
    {
        aGroups.push_back(toRanges(rGroup.second));
        std::sort(aGroups.back().begin(), aGroups.back().end());
    }
    std::sort(aGroups.begin(), aGroups.end());
    return aGroups;
}
}

// The inventory of a sheet holds every styled table and every sparkline group on it, wherever the
// cursor is, so that the client can keep them and decide what to do with them itself.
CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testCellRangeInventory)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    ScTestViewCallback aView1;
    KitHelper::createView();
    ScTestViewCallback aView2;

    // Returns all the invalidations a view received since the last call,
    // sorted and joined to make it deterministic.  For now,
    auto takeInvalidations = [](ScTestViewCallback& rView) {
        auto& rParts = rView.m_aCellRangeInventoryInvalidations;
        std::sort(rParts.begin(), rParts.end());
        OString aJoined;
        for (const OString& rPart : rParts)
            aJoined += (aJoined.isEmpty() ? "" : ", ") + rPart;
        rParts.clear();
        return aJoined;
    };

    // Nothing on the sheet yet, but every kind is there.  This first pull also starts the
    // invalidations.
    boost::property_tree::ptree aTree
        = getCommandResponse(*pModelObj, ".uno:CellRangeInventory?part=0");
    // {"commandName": ".uno:CellRangeInventory",
    //  "commandValues": {"part": 0, "tables": [], "sparklineGroups": [], "mergedCells": []}}
    CPPUNIT_ASSERT_EQUAL(std::string(".uno:CellRangeInventory"),
                         aTree.get<std::string>("commandName"));
    CPPUNIT_ASSERT_EQUAL(0, aTree.get<int>("commandValues.part"));
    CPPUNIT_ASSERT(aTree.get_child("commandValues.tables").empty());
    CPPUNIT_ASSERT(aTree.get_child("commandValues.sparklineGroups").empty());
    CPPUNIT_ASSERT(aTree.get_child("commandValues.mergedCells").empty());

    // Insert a styled table in A1:C4.  Every view should be notified of the
    // change.
    ScDBDocFunc aFunc(*pDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 3, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView1));
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView2));

    // Insert a sparkline group in E1:E2 and another one in G5 and G7.
    ScDocFunc& rDocFunc = pDocShell->GetDocFunc();
    auto pGroup1 = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(ScRange(0, 0, 0, 2, 1, 0), ScRange(4, 0, 0, 4, 1, 0),
                                             pGroup1)); // E1:E2
    auto pGroup2 = std::make_shared<sc::SparklineGroup>();
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(ScRange(0, 4, 0, 2, 4, 0), ScRange(6, 4, 0, 6, 4, 0),
                                             pGroup2)); // G5
    CPPUNIT_ASSERT(rDocFunc.InsertSparklines(ScRange(0, 6, 0, 2, 6, 0), ScRange(6, 6, 0, 6, 6, 0),
                                             pGroup2)); // G7
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL("part=0, part=0, part=0"_ostr, takeInvalidations(aView1));
    CPPUNIT_ASSERT_EQUAL("part=0, part=0, part=0"_ostr, takeInvalidations(aView2));

    // The cursor is in none of them.  A cursor move is no change to the inventory.
    pView->SetCursor(9, 9); // J10
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(""_ostr, takeInvalidations(aView1));
    CPPUNIT_ASSERT_EQUAL(""_ostr, takeInvalidations(aView2));

    aTree = getCommandResponse(*pModelObj, ".uno:CellRangeInventory?part=0");
    CPPUNIT_ASSERT_EQUAL(0, aTree.get<int>("commandValues.part"));

    const boost::property_tree::ptree& rTables = aTree.get_child("commandValues.tables");
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), rTables.size());
    const ScRange aExpectedTable(0, 0, 0, 2, 3, 0);
    CPPUNIT_ASSERT_EQUAL(aExpectedTable, toRange(rTables.front().second.get_child("range")));
    CPPUNIT_ASSERT(rTables.front().second.get<bool>("editable"));

    // Neighboring cells of a group are combined, but separate ones are not.
    std::vector<std::vector<ScRange>> aExpectedGroups
        // group1: E1:E2, group2: G5, G7
        = { { ScRange(4, 0, 0, 4, 1, 0) }, { ScRange(6, 4, 0), ScRange(6, 6, 0) } };
    assertSparklineGroupsEqual(aExpectedGroups, getSparklineGroups(*pModelObj));

    // Insert a row above G5 to move group2 down by one row, below the table.
    CPPUNIT_ASSERT(rDocFunc.InsertCells(ScRange(0, 4, 0, pDoc->MaxCol(), 4, 0), nullptr,
                                        INS_INSROWS_BEFORE, /*bRecord*/ true, /*bApi*/ true));
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView1));
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView2));
    // group1: E1:E2, group2: G6, G8
    aExpectedGroups = { { ScRange(4, 0, 0, 4, 1, 0) }, { ScRange(6, 5, 0), ScRange(6, 7, 0) } };
    assertSparklineGroupsEqual(aExpectedGroups, getSparklineGroups(*pModelObj));

    // The cursor-driven marker of the group under the cursor should reflect the
    // shifting of the cells from above.
    aView1.m_aCellRangeMarkers.clear();
    pView->SetCursor(6, 5); // G6
    Scheduler::ProcessEventsToIdle();
    auto itMarker = std::find_if(
        aView1.m_aCellRangeMarkers.begin(), aView1.m_aCellRangeMarkers.end(),
        [](const boost::property_tree::ptree& rMarker) {
            return rMarker.get<std::string>("state.name") == "SparklineGroup";
        });
    CPPUNIT_ASSERT(itMarker != aView1.m_aCellRangeMarkers.end());
    std::vector<std::string> aMarkerRanges = getCellRanges(itMarker->get_child("state.cellRanges"));
    std::sort(aMarkerRanges.begin(), aMarkerRanges.end());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), aMarkerRanges.size());
    CPPUNIT_ASSERT_EQUAL(std::string("6, 5, 6, 5"), aMarkerRanges[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("6, 7, 6, 7"), aMarkerRanges[1]);
    pView->SetCursor(9, 9); // J10
    Scheduler::ProcessEventsToIdle();

    // Ungrouping G6:G8 gives each of its sparklines a group of its own.
    CPPUNIT_ASSERT(rDocFunc.UngroupSparklines(ScRange(6, 5, 0, 6, 7, 0)));
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView1));
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView2));
    // group1: E1:E2, group2: G6, group3: G8
    aExpectedGroups = { { ScRange(4, 0, 0, 4, 1, 0) }, { ScRange(6, 5, 0) }, { ScRange(6, 7, 0) } };
    assertSparklineGroupsEqual(aExpectedGroups, getSparklineGroups(*pModelObj));

    // E1 pasted into I1:I2 adds both cells to group1.
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$E$1"_ustr) });
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    { comphelper::makePropertyValue(u"ToPoint"_ustr, u"$I$1:$I$2"_ustr) });
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView1));
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView2));
    // group1: E1:E2, I1:I2, group2: G6, group3: G8
    aExpectedGroups = { { ScRange(4, 0, 0, 4, 1, 0), ScRange(8, 0, 0, 8, 1, 0) },
                        { ScRange(6, 5, 0) },
                        { ScRange(6, 7, 0) } };
    assertSparklineGroupsEqual(aExpectedGroups, getSparklineGroups(*pModelObj));
    pView->SetCursor(9, 9); // J10
    Scheduler::ProcessEventsToIdle();

    // Merge L1:M2 and L4:N4.  Each merge is an inventory change.
    ScCellMergeOption aMerge1(11, 0, 12, 1); // L1:M2
    aMerge1.maTabs.insert(0);
    CPPUNIT_ASSERT(rDocFunc.MergeCells(aMerge1, false, true, true));
    ScCellMergeOption aMerge2(11, 3, 13, 3); // L4:N4
    aMerge2.maTabs.insert(0);
    CPPUNIT_ASSERT(rDocFunc.MergeCells(aMerge2, false, true, true));
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL("part=0, part=0"_ostr, takeInvalidations(aView1));
    CPPUNIT_ASSERT_EQUAL("part=0, part=0"_ostr, takeInvalidations(aView2));
    aTree = getCommandResponse(*pModelObj, ".uno:CellRangeInventory?part=0");
    // "mergedCells": [[11, 0, 12, 1], [11, 3, 13, 3]]
    std::vector<ScRange> aMergedCells = toRanges(aTree.get_child("commandValues.mergedCells"));
    std::vector<ScRange> aExpectedMergedCells
        = { ScRange(11, 0, 0, 12, 1, 0), ScRange(11, 3, 0, 13, 3, 0) };
    assertRangesEqual(aExpectedMergedCells, aMergedCells);

    // Unmerge L1:M2, which leaves L4:N4.
    CPPUNIT_ASSERT(rDocFunc.UnmergeCells(aMerge1, true, nullptr));
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView1));
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView2));
    aTree = getCommandResponse(*pModelObj, ".uno:CellRangeInventory?part=0");
    aMergedCells = toRanges(aTree.get_child("commandValues.mergedCells"));
    aExpectedMergedCells = { ScRange(11, 3, 0, 13, 3, 0) };
    assertRangesEqual(aExpectedMergedCells, aMergedCells);

    // A protected sheet makes the table non-editable, which is still listed,
    // and is an inventory change.
    ScTableProtection aProtection;
    aProtection.setProtected(true);
    pDocShell->GetDocFunc().ProtectSheet(0, aProtection);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView1));
    CPPUNIT_ASSERT_EQUAL("part=0"_ostr, takeInvalidations(aView2));
    aTree = getCommandResponse(*pModelObj, ".uno:CellRangeInventory?part=0");
    const boost::property_tree::ptree& rLockedTables = aTree.get_child("commandValues.tables");
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), rLockedTables.size());
    CPPUNIT_ASSERT(!rLockedTables.front().second.get<bool>("editable"));

    // A sheet inserted in front moves the table to part 1, so both parts changed.
    CPPUNIT_ASSERT(pDocShell->GetDocFunc().InsertTable(0, u"Sheet0"_ustr, /*bRecord*/ true,
                                                       /*bApi*/ true));
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL("part=0, part=1"_ostr, takeInvalidations(aView1));
    CPPUNIT_ASSERT_EQUAL("part=0, part=1"_ostr, takeInvalidations(aView2));
    aTree = getCommandResponse(*pModelObj, ".uno:CellRangeInventory?part=1");
    CPPUNIT_ASSERT_EQUAL(1, aTree.get<int>("commandValues.part"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), aTree.get_child("commandValues.tables").size());

    // The new sheet has its own, empty, inventory.  Other parameters are ignored.
    aTree = getCommandResponse(*pModelObj, ".uno:CellRangeInventory?foo=bar&part=0");
    CPPUNIT_ASSERT_EQUAL(0, aTree.get<int>("commandValues.part"));
    CPPUNIT_ASSERT(aTree.get_child("commandValues.tables").empty());
    CPPUNIT_ASSERT(aTree.get_child("commandValues.sparklineGroups").empty());
    CPPUNIT_ASSERT(aTree.get_child("commandValues.mergedCells").empty());

    // No values for a part that is not a sheet, or without one.
    aTree = getCommandResponse(*pModelObj, ".uno:CellRangeInventory?part=5");
    CPPUNIT_ASSERT(!aTree.get_child_optional("commandValues"));
    aTree = getCommandResponse(*pModelObj, ".uno:CellRangeInventory");
    CPPUNIT_ASSERT(!aTree.get_child_optional("commandValues"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
