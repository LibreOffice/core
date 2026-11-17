/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sctiledrenderingtest.hxx>
#include <qahelper.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/awt/Key.hpp>

#include <COKit/COKit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/kit/helper.hxx>
#include <sfx2/viewsh.hxx>
#include <tools/json_writer.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/scheduler.hxx>

#include <cellmergeoption.hxx>
#include <dbdocfun.hxx>
#include <docfunc.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <docuno.hxx>
#include <global.hxx>
#include <gridwin.hxx>
#include <markdata.hxx>
#include <sctestviewcallback.hxx>
#include <SparklineGroup.hxx>
#include <tabprotection.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>

using namespace com::sun::star;

namespace
{
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
 * Read a cell sent as [col, row] into an address on sheet 0.
 */
ScAddress toAddress(const boost::property_tree::ptree& rArray)
{
    std::vector<sal_Int32> aIndexes = toIntegers(rArray);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), aIndexes.size());
    return ScAddress(aIndexes[0], aIndexes[1], 0);
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
 * Read a rectangle sent as "x, y, width, height".
 */
tools::Rectangle toRectangle(const OString& rPayload)
{
    std::stringstream aStream{ std::string(rPayload) };
    tools::Long nX = 0, nY = 0, nWidth = 0, nHeight = 0;
    char cComma = 0;
    aStream >> nX >> cComma >> nY >> cComma >> nWidth >> cComma >> nHeight;
    CPPUNIT_ASSERT(!aStream.fail());
    return tools::Rectangle(Point(nX, nY), Size(nWidth, nHeight));
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

// The client sets the cell cursor and the selection of one view, and the
// other views are told.
CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testSetKitCellCursor)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;
    ScTabViewShell* pView1 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1);
    KitHelper::createView();
    ScTestViewCallback aView2;
    ScTabViewShell* pView2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView2);
    KitHelper::setView(nView1);
    Scheduler::ProcessEventsToIdle();

    // Call the view through SfxViewShell, as the kit command does.
    SfxViewShell& rShell1 = *pView1;
    ScViewData& rViewData1 = pView1->GetViewData();
    ScViewData& rViewData2 = pView2->GetViewData();

    // Select A2:C5 with the anchor in A2 and the moving end in C5.  The cell
    // cursor should stay in A2 as with Shift+arrow.
    aView1.m_bOwnCursorInvalidated = false;
    aView2.m_bViewCursorInvalidated = false;
    SfxCellPosition aCell{ 2, 4 }; // C5
    SfxCellPosition aAnchor{ 0, 1 }; // A2
    rShell1.setKitCellCursor(0, aCell, &aAnchor);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), rViewData1.GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(1), rViewData1.GetCurY());
    CPPUNIT_ASSERT(rViewData1.GetMarkData().IsMarked());
    // A2:C5 should be selected
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 2, 4, 0), rViewData1.GetMarkData().GetMarkArea());
    CPPUNIT_ASSERT(aView1.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(aView2.m_bViewCursorInvalidated);

    // Read the state back the way kit does after the command, which sets
    // the view first.
    // {"commandName": ".uno:CellCursorState",
    //  "commandValues": {"part": 0, "cursor": {"cell": [0, 1], "visible": true},
    //                    "selection": [[0, 1, 2, 4]]}}
    KitHelper::setView(nView1);
    boost::property_tree::ptree aTree = getCommandResponse(*pModelObj, ".uno:CellCursorState");
    CPPUNIT_ASSERT_EQUAL(0, aTree.get<int>("commandValues.part"));
    const ScAddress aExpectedAnchor(0, 1, 0);
    CPPUNIT_ASSERT_EQUAL(aExpectedAnchor, toAddress(aTree.get_child("commandValues.cursor.cell")));
    CPPUNIT_ASSERT(aTree.get<bool>("commandValues.cursor.visible"));
    std::vector<ScRange> aSelection = toRanges(aTree.get_child("commandValues.selection"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), aSelection.size());
    const ScRange aExpectedSelection(0, 1, 0, 2, 4, 0);
    CPPUNIT_ASSERT_EQUAL(aExpectedSelection, aSelection[0]);

    // Press Shift+Down to extend the same selection from A2:C5 to A2:C6.
    // Set the view first, as kit does before each input.
    KitHelper::setView(nView1);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::DOWN | KEY_SHIFT);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::DOWN | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();
    // A2:C6 should be selected
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 2, 5, 0), rViewData1.GetMarkData().GetMarkArea());

    // Check that the view receives the selection as cell ranges.
    // {"part": 0, "ranges": [[0, 1, 2, 5]]}
    CPPUNIT_ASSERT_EQUAL(0, aView1.m_aCellSelection.get<int>("part"));
    aSelection = toRanges(aView1.m_aCellSelection.get_child("ranges"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), aSelection.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 2, 5, 0), aSelection[0]);

    // Check that the other view hears of the same selection.
    // {"viewId": "0", "part": "0", "mode": "0", "ranges": [[0, 1, 2, 5]]}
    CPPUNIT_ASSERT_EQUAL(nView1, aView2.m_aCellViewSelection.get<int>("viewId"));
    aSelection = toRanges(aView2.m_aCellViewSelection.get_child("ranges"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), aSelection.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 1, 0, 2, 5, 0), aSelection[0]);

    // Check that the other view keeps its own cursor in A1.
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), rViewData2.GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(0), rViewData2.GetCurY());

    // Select E3 alone by putting the anchor on the cursor cell, which differs
    // from a cell cursor with no selection.
    aCell = { 4, 2 }; // E3
    rShell1.setKitCellCursor(0, aCell, &aCell);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(SCCOL(4), rViewData1.GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(2), rViewData1.GetCurY());
    CPPUNIT_ASSERT(rViewData1.GetMarkData().IsMarked());
    // E3 is selected and marked
    CPPUNIT_ASSERT_EQUAL(ScRange(4, 2, 0, 4, 2, 0), rViewData1.GetMarkData().GetMarkArea());

    // Move the cursor to D7 with no anchor, which should clear the selection.
    aCell = { 3, 6 }; // D7
    rShell1.setKitCellCursor(0, aCell, nullptr);
    Scheduler::ProcessEventsToIdle();
    // cell cursor is on D7 but no selection
    CPPUNIT_ASSERT_EQUAL(SCCOL(3), rViewData1.GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), rViewData1.GetCurY());
    CPPUNIT_ASSERT(!rViewData1.GetMarkData().IsMarked());
    CPPUNIT_ASSERT(!rViewData1.GetMarkData().IsMultiMarked());
    // {"part": 0, "ranges": []}
    CPPUNIT_ASSERT(aView1.m_aCellSelection.get_child("ranges").empty());
    CPPUNIT_ASSERT(aView2.m_aCellViewSelection.get_child("ranges").empty());
    KitHelper::setView(nView1);
    aTree = getCommandResponse(*pModelObj, ".uno:CellCursorState");
    const ScAddress aExpectedCursor(3, 6, 0);
    CPPUNIT_ASSERT_EQUAL(aExpectedCursor, toAddress(aTree.get_child("commandValues.cursor.cell")));
    CPPUNIT_ASSERT(aTree.get_child("commandValues.selection").empty());

    // Attempt to set a cell cursor on a non-current sheet should be ignored.
    aCell = { 0, 0 };
    rShell1.setKitCellCursor(1, aCell, nullptr);
    CPPUNIT_ASSERT_EQUAL(SCCOL(3), rViewData1.GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), rViewData1.GetCurY());
    CPPUNIT_ASSERT(!rViewData1.GetMarkData().IsMarked());

    // Attempt to set a cell cursor outside the sheet should be ignored.
    aCell = { -1, 0 };
    rShell1.setKitCellCursor(0, aCell, nullptr);
    CPPUNIT_ASSERT_EQUAL(SCCOL(3), rViewData1.GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), rViewData1.GetCurY());
    CPPUNIT_ASSERT(!rViewData1.GetMarkData().IsMarked());

    // Attempt to select from an anchor outside the sheet should be ignored.
    aCell = { 0, 0 };
    aAnchor = { -1, 0 };
    rShell1.setKitCellCursor(0, aCell, &aAnchor);
    CPPUNIT_ASSERT_EQUAL(SCCOL(3), rViewData1.GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(6), rViewData1.GetCurY());
    CPPUNIT_ASSERT(!rViewData1.GetMarkData().IsMarked());

    // Read the state back after the refused moves, which the client uses to
    // put its own cursor back.
    KitHelper::setView(nView1);
    aTree = getCommandResponse(*pModelObj, ".uno:CellCursorState");
    CPPUNIT_ASSERT_EQUAL(aExpectedCursor, toAddress(aTree.get_child("commandValues.cursor.cell")));
    CPPUNIT_ASSERT(aTree.get_child("commandValues.selection").empty());

    // Merge A1:C1 and move the cursor to B1, which the merge covers.  The
    // cursor should go to A1, where the merged cell starts.
    ScDocument* pDoc = pModelObj->GetDocument();
    pDoc->DoMerge(0, 0, 2, 0, 0); // A1:C1
    aCell = { 1, 0 }; // B1
    rShell1.setKitCellCursor(0, aCell, nullptr);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), rViewData1.GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(0), rViewData1.GetCurY());

    // Select from the anchor B1 to D3.  The anchor and the cursor should go
    // to A1.
    aCell = { 3, 2 }; // D3
    aAnchor = { 1, 0 }; // B1
    rShell1.setKitCellCursor(0, aCell, &aAnchor);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), rViewData1.GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(0), rViewData1.GetCurY());
    // A1:D3 should be selected
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 3, 2, 0), rViewData1.GetMarkData().GetMarkArea());
}

// A view with the local cell cursor on receives its selection as cell ranges
// only, and no area of the fill handle.
CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testLocalCellCursorMode)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScTestViewCallback aView;
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    Scheduler::ProcessEventsToIdle();

    // Call the view through SfxViewShell, as the kit command does.
    SfxViewShell& rShell = *pView;

    // Start with the local cell cursor off
    rShell.setKitLocalCellCursor(false);

    // Select A1:D3.
    SfxCellPosition aCell{ 3, 2 }; // D3
    SfxCellPosition aAnchor{ 0, 0 }; // A1
    rShell.setKitCellCursor(0, aCell, &aAnchor);
    Scheduler::ProcessEventsToIdle();
    std::vector<ScRange> aSelection = toRanges(aView.m_aCellSelection.get_child("ranges"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), aSelection.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 3, 2, 0), aSelection[0]);
    // For a single selection, both textselection and cellselectionarea should
    // contain the same rectangle.
    CPPUNIT_ASSERT(!aView.m_aTextSelections.empty());
    CPPUNIT_ASSERT(!aView.m_aCellSelectionAreas.empty());
    const tools::Rectangle aSelectionRect = toRectangle(aView.m_aTextSelections.back());
    CPPUNIT_ASSERT_EQUAL(Point(0, 0), aSelectionRect.TopLeft());
    CPPUNIT_ASSERT(!aSelectionRect.IsEmpty());
    CPPUNIT_ASSERT_EQUAL(aSelectionRect, toRectangle(aView.m_aCellSelectionAreas.back()));
    // The view should also receive the area of the fill handle, which sits on
    // the bottom-right corner of the selection.
    CPPUNIT_ASSERT(!aView.m_aCellAutoFillAreas.empty());
    const tools::Rectangle aFillRect = toRectangle(aView.m_aCellAutoFillAreas.back());
    CPPUNIT_ASSERT(aFillRect.Contains(aSelectionRect.BottomRight()));
    // The cell cursor state is only for a view with the local cell cursor on.
    CPPUNIT_ASSERT(aView.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(aView.m_aCellCursorState.empty());

    // Switch the view to the local cell cursor and select B5:C6.  The view
    // should receive the cursor and the selection as one cell cursor state,
    // and no rectangles, cellcursor, cellselection or area of the fill handle.
    rShell.setKitLocalCellCursor(true);
    aView.m_bOwnCursorInvalidated = false;
    aView.m_aCellSelection.clear();
    aView.m_aTextSelections.clear();
    aView.m_aCellSelectionAreas.clear();
    aView.m_aCellAutoFillAreas.clear();
    aCell = { 2, 5 }; // C6
    aAnchor = { 1, 4 }; // B5
    rShell.setKitCellCursor(0, aCell, &aAnchor);
    Scheduler::ProcessEventsToIdle();
    // {"part": 0, "cursor": {"cell": [1, 4], "visible": true},
    //  "selection": [[1, 4, 2, 5]]}
    CPPUNIT_ASSERT_EQUAL(0, aView.m_aCellCursorState.get<int>("part"));
    CPPUNIT_ASSERT_EQUAL(ScAddress(1, 4, 0),
                         toAddress(aView.m_aCellCursorState.get_child("cursor.cell")));
    CPPUNIT_ASSERT(aView.m_aCellCursorState.get<bool>("cursor.visible"));
    aSelection = toRanges(aView.m_aCellCursorState.get_child("selection"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), aSelection.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 4, 0, 2, 5, 0), aSelection[0]);
    CPPUNIT_ASSERT(!aView.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(aView.m_aCellSelection.empty());
    CPPUNIT_ASSERT(aView.m_aTextSelections.empty());
    CPPUNIT_ASSERT(aView.m_aCellSelectionAreas.empty());
    CPPUNIT_ASSERT(aView.m_aCellAutoFillAreas.empty());

    // Move the cursor to D7 to clear the selection.
    aCell = { 3, 6 }; // D7
    rShell.setKitCellCursor(0, aCell, nullptr);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(ScAddress(3, 6, 0),
                         toAddress(aView.m_aCellCursorState.get_child("cursor.cell")));
    CPPUNIT_ASSERT(aView.m_aCellCursorState.get_child("selection").empty());
    CPPUNIT_ASSERT(!aView.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(aView.m_aCellSelection.empty());
    CPPUNIT_ASSERT(aView.m_aTextSelections.empty());
    CPPUNIT_ASSERT(aView.m_aCellSelectionAreas.empty());
    CPPUNIT_ASSERT(aView.m_aCellAutoFillAreas.empty());

    // Hide the cell cursor, then show it again.  The state should report
    // each change.
    ScGridWindow* pGridWindow = pView->GetViewData().GetActiveWin();
    CPPUNIT_ASSERT(pGridWindow);
    pGridWindow->DeleteCursorOverlay();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView.m_aCellCursorState.get<bool>("cursor.visible"));
    pGridWindow->UpdateCursorOverlay();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_aCellCursorState.get<bool>("cursor.visible"));
}

// A view with the local cell cursor on receives the selections of the other
// views as cell ranges only.
CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testLocalCellCursorOtherViews)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;
    ScTabViewShell* pView1 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1);
    KitHelper::createView();
    int nView2 = KitHelper::getCurrentView();
    ScTestViewCallback aView2;
    ScTabViewShell* pView2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView2);
    Scheduler::ProcessEventsToIdle();

    // Call the views through SfxViewShell, as the kit command does.
    SfxViewShell& rShell1 = *pView1;
    SfxViewShell& rShell2 = *pView2;

    // Start with the local cell cursor off in both views.
    rShell1.setKitLocalCellCursor(false);
    rShell2.setKitLocalCellCursor(false);

    // Select A1:D3 in view 1.  View 2 should receive it both as rectangles
    // and as cell ranges.
    aView2.m_bTextViewSelectionInvalidated = false;
    KitHelper::setView(nView1);
    SfxCellPosition aCell{ 3, 2 }; // D3
    SfxCellPosition aAnchor{ 0, 0 }; // A1
    rShell1.setKitCellCursor(0, aCell, &aAnchor);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView2.m_bTextViewSelectionInvalidated);
    std::vector<ScRange> aSelection = toRanges(aView2.m_aCellViewSelection.get_child("ranges"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), aSelection.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 3, 2, 0), aSelection[0]);

    // Switch view 2 to the local cell cursor and select B5:C6 in view 1.
    // View 2 should receive it as cell ranges only.
    rShell2.setKitLocalCellCursor(true);
    aView2.m_bTextViewSelectionInvalidated = false;
    aCell = { 2, 5 }; // C6
    aAnchor = { 1, 4 }; // B5
    rShell1.setKitCellCursor(0, aCell, &aAnchor);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView2.m_bTextViewSelectionInvalidated);
    aSelection = toRanges(aView2.m_aCellViewSelection.get_child("ranges"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), aSelection.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 4, 0, 2, 5, 0), aSelection[0]);

    // Move the cursor of view 1 to D7 to clear its selection.
    aCell = { 3, 6 }; // D7
    rShell1.setKitCellCursor(0, aCell, nullptr);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView2.m_bTextViewSelectionInvalidated);
    CPPUNIT_ASSERT(aView2.m_aCellViewSelection.get_child("ranges").empty());

    // Select E2:F3 in view 2.  View 1, which has the local cell cursor off,
    // should still receive it both as rectangles and as cell ranges.
    aView1.m_bTextViewSelectionInvalidated = false;
    KitHelper::setView(nView2);
    aCell = { 5, 2 }; // F3
    aAnchor = { 4, 1 }; // E2
    rShell2.setKitCellCursor(0, aCell, &aAnchor);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bTextViewSelectionInvalidated);
    aSelection = toRanges(aView1.m_aCellViewSelection.get_child("ranges"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), aSelection.size());
    CPPUNIT_ASSERT_EQUAL(ScRange(4, 1, 0, 5, 2, 0), aSelection[0]);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
