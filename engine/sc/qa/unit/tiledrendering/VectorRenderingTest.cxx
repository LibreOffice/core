/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sctiledrenderingtest.hxx>
#include <sfx2/kit/helper.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <attrib.hxx>
#include <cellmergeoption.hxx>
#include <columnspanset.hxx>
#include <docfunc.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <docuno.hxx>
#include <drwlayer.hxx>
#include <globstr.hrc>
#include <gridwin.hxx>
#include <markdata.hxx>
#include <scitems.hxx>
#include <scresid.hxx>
#include <Sparkline.hxx>
#include <SparklineGroup.hxx>
#include <tabvwsh.hxx>
#include <VectorContent.hxx>

#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/intitem.hxx>
#include <svl/numformat.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdpage.hxx>
#include <tools/json_writer.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/virdev.hxx>

#include <o3tl/unit_conversion.hxx>
#include <osl/process.h>

#include <cmath>
#include <cstdlib>
#include <functional>
#include <optional>
#include <set>
#include <sstream>
#include <vector>

namespace
{
boost::property_tree::ptree parseJson(const OString& rJson)
{
    std::stringstream aStream(std::string(rJson.getStr(), rJson.getLength()));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    return aTree;
}

/// Ask the document for vector primitives, as a client does.
boost::property_tree::ptree requestVectorPrimitives(ScModelObj* pModelObj, std::string_view aQuery)
{
    tools::JsonWriter aWriter;
    const OString aCommand = OString::Concat(".uno:VectorPrimitives?") + aQuery;
    static_cast<vcl::ITiledRenderable*>(pModelObj)->getCommandValues(aWriter, aCommand);
    return parseJson(aWriter.finishAndGetAsOString());
}

/// The rows of every block in a response.
std::vector<boost::property_tree::ptree> blockRows(const boost::property_tree::ptree& rResponse)
{
    std::vector<boost::property_tree::ptree> aRows;
    for (const auto& rObject : rResponse.get_child("objects"))
    {
        if (rObject.second.get<std::string>("kind") != "cells")
            continue;
        for (const auto& rRow : rObject.second.get_child("rows"))
            aRows.push_back(rRow.second);
    }
    return aRows;
}

/// The cell text primitives of the text layer of every row of every block in a response.
std::vector<boost::property_tree::ptree> cellTexts(const boost::property_tree::ptree& rResponse)
{
    std::vector<boost::property_tree::ptree> aTexts;
    for (const auto& rRow : blockRows(rResponse))
    {
        auto oText = rRow.get_child_optional("content.layers.text");
        if (!oText)
            continue;
        for (const auto& rPrimitive : *oText)
        {
            if (rPrimitive.second.get<std::string>("type") == "calcCellText")
                aTexts.push_back(rPrimitive.second);
        }
    }
    return aTexts;
}

std::optional<boost::property_tree::ptree> cellTextAt(const boost::property_tree::ptree& rResponse,
                                                      int nColumn, int nRow)
{
    for (const auto& rText : cellTexts(rResponse))
    {
        if (rText.get<int>("column") == nColumn && rText.get<int>("row") == nRow)
            return rText;
    }
    return std::nullopt;
}

/// The flags of a cell text primitive, by name.
std::set<std::string> cellTextFlags(const boost::property_tree::ptree& rText)
{
    std::set<std::string> aFlags;
    if (auto oFlags = rText.get_child_optional("flags"))
    {
        for (const auto& rFlag : *oFlags)
            aFlags.insert(rFlag.second.get_value<std::string>());
    }
    return aFlags;
}

/// Whether the marks layer of any row carries a mark of the given kind on the given cell.
bool hasCellMark(const boost::property_tree::ptree& rResponse, std::string_view aKind, int nColumn,
                 int nRow)
{
    for (const auto& rRow : blockRows(rResponse))
    {
        auto oMarks = rRow.get_child_optional("content.layers.marks");
        if (!oMarks)
            continue;
        for (const auto& rPrimitive : *oMarks)
        {
            if (rPrimitive.second.get<std::string>("type") == "calcCellMark"
                && rPrimitive.second.get<std::string>("kind") == aKind
                && rPrimitive.second.get<int>("column") == nColumn
                && rPrimitive.second.get<int>("row") == nRow)
                return true;
        }
    }
    return false;
}

/// The border layer of the first block in a response that has borders, or an empty tree.
boost::property_tree::ptree blockBorders(const boost::property_tree::ptree& rResponse)
{
    for (const auto& rObject : rResponse.get_child("objects"))
    {
        if (rObject.second.get<std::string>("kind") != "cells")
            continue;
        if (auto oBorders = rObject.second.get_child_optional("borders.content.layers.borders"))
            return *oBorders;
    }
    return boost::property_tree::ptree();
}

/// The areas of the grid covers of every row, each as start column, start row, end column and
/// end row.
std::vector<std::vector<int>> gridCoverAreas(const boost::property_tree::ptree& rResponse)
{
    std::vector<std::vector<int>> aAreas;
    for (const auto& rRow : blockRows(rResponse))
    {
        auto oCovers = rRow.get_child_optional("content.layers.gridCover");
        if (!oCovers)
            continue;
        for (const auto& rCover : *oCovers)
        {
            for (const auto& rArea : rCover.second.get_child("areas"))
                aAreas.push_back({ rArea.second.get<int>("startColumn"),
                                   rArea.second.get<int>("startRow"),
                                   rArea.second.get<int>("endColumn"),
                                   rArea.second.get<int>("endRow") });
        }
    }
    return aAreas;
}

int blockCount(const boost::property_tree::ptree& rResponse)
{
    int nCount = 0;
    for (const auto& rObject : rResponse.get_child("objects"))
    {
        if (rObject.second.get<std::string>("kind") == "cells")
            ++nCount;
    }
    return nCount;
}

sal_Int64 blockVersion(const boost::property_tree::ptree& rResponse)
{
    for (const auto& rObject : rResponse.get_child("objects"))
    {
        if (rObject.second.get<std::string>("kind") == "cells")
            return rObject.second.get<sal_Int64>("version");
    }
    return -1;
}

/// The entry of the drawing object with the given id in a response.
std::optional<boost::property_tree::ptree>
drawingEntry(const boost::property_tree::ptree& rResponse, sal_uInt64 nId)
{
    for (const auto& rBlock : rResponse.get_child("objects"))
    {
        if (rBlock.second.get<std::string>("kind") != "cells")
            continue;
        for (const auto& rObject : rBlock.second.get_child("objects"))
        {
            if (rObject.second.get<std::string>("kind") == "drawing"
                && rObject.second.get<sal_uInt64>("id") == nId)
                return rObject.second;
        }
    }
    return std::nullopt;
}

void setCellString(ScDocShell& rDocShell, SCCOL nColumn, SCROW nRow, const OUString& rText)
{
    rDocShell.GetDocFunc().SetStringCell(ScAddress(nColumn, nRow, 0), rText, true);
    Scheduler::ProcessEventsToIdle();
}

void setCellValue(ScDocShell& rDocShell, SCCOL nColumn, SCROW nRow, double fValue)
{
    rDocShell.GetDocFunc().SetValueCell(ScAddress(nColumn, nRow, 0), fValue, true);
    Scheduler::ProcessEventsToIdle();
}

/// Apply a pattern to one cell of the first sheet and repaint.
void applyToCell(ScDocShell& rDocShell, SCCOL nColumn, SCROW nRow, const ScPatternAttr& rPattern)
{
    rDocShell.GetDocument().ApplyPatternAreaTab(nColumn, nRow, nColumn, nRow, 0, rPattern);
    rDocShell.PostPaintGridAll();
    Scheduler::ProcessEventsToIdle();
}

void clearCell(ScDocShell& rDocShell, SCCOL nColumn, SCROW nRow)
{
    ScMarkData aMark(rDocShell.GetDocument().GetSheetLimits());
    aMark.SetMarkArea(ScRange(nColumn, nRow, 0));
    rDocShell.GetDocFunc().DeleteContents(aMark, InsertDeleteFlags::CONTENTS, true, true);
    Scheduler::ProcessEventsToIdle();
}

/// Paint one tile of 256 by 256 pixels at 100% zoom and return its pixels.
std::vector<unsigned char> paintTile(ScModelObj* pModelObj)
{
    constexpr int nCanvasSize = 256;
    std::vector<unsigned char> aBuffer(nCanvasSize * nCanvasSize * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixelScaleOffsetAndKitBuffer(Size(nCanvasSize, nCanvasSize), 1.0, Point(),
                                                       aBuffer.data());
    pModelObj->paintTile(*pDevice, nCanvasSize, nCanvasSize, /*nTilePosX=*/0, /*nTilePosY=*/0,
                         /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    Scheduler::ProcessEventsToIdle();
    return aBuffer;
}

/// The number of pixels whose color differs by more than a small amount in any channel.
int differingPixels(const std::vector<unsigned char>& rFirst,
                    const std::vector<unsigned char>& rSecond)
{
    int nDiffering = 0;
    for (size_t i = 0; i + 3 < rFirst.size(); i += 4)
    {
        for (size_t nChannel = 0; nChannel < 3; ++nChannel)
        {
            if (std::abs(int(rFirst[i + nChannel]) - int(rSecond[i + nChannel])) > 16)
            {
                ++nDiffering;
                break;
            }
        }
    }
    return nDiffering;
}
}

namespace
{
/// The JSON of part of an answer, to compare it with the same part of another.
std::string asJson(const boost::property_tree::ptree& rTree)
{
    std::ostringstream aStream;
    boost::property_tree::write_json(aStream, rTree, false);
    return aStream.str();
}

void setColumnWidth(ScDocShell& rDocShell, SCCOL nColumn, sal_uInt16 nTwips)
{
    const std::vector<sc::ColRowSpan> aSpans{ sc::ColRowSpan(nColumn, nColumn) };
    rDocShell.GetDocFunc().SetWidthOrHeight(true, aSpans, 0, SC_SIZE_DIRECT, nTwips, false, true);
    Scheduler::ProcessEventsToIdle();
}

/// The values of a position written relative to the cells, [cell, offset].
std::pair<int, double> cellPosition(const boost::property_tree::ptree& rPosition)
{
    std::vector<double> aValues;
    for (const auto& rValue : rPosition)
        aValues.push_back(rValue.second.get_value<double>());
    CPPUNIT_ASSERT_EQUAL(size_t(2), aValues.size());
    return { int(aValues[0]), aValues[1] };
}
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorCellPositionsKeepOtherColumns)
{
    // With positions relative to the cells, the content of a cell does not change when a column
    // before it gets wider.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 3, 1, u"Hello"_ustr);
    Scheduler::ProcessEventsToIdle();

    const auto oBefore = cellTextAt(
        requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0"), 3, 1);
    CPPUNIT_ASSERT(oBefore);

    setColumnWidth(*pDocShell, 0, 3000);

    const auto oAfter = cellTextAt(
        requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0"), 3, 1);
    CPPUNIT_ASSERT(oAfter);
    CPPUNIT_ASSERT_EQUAL(asJson(*oBefore), asJson(*oAfter));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorBlockHoldsCellText)
{
    // A block carries the text of each cell with the cell it belongs to and its alignment.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 0, 0, u"Hello"_ustr);
    pDocShell->GetDocFunc().SetValueCell(ScAddress(1, 1, 0), 123.0, true);
    Scheduler::ProcessEventsToIdle();

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");

    CPPUNIT_ASSERT_EQUAL(std::string("vectorprimitives"), aResponse.get<std::string>("type"));
    CPPUNIT_ASSERT_EQUAL(std::string("spreadsheet"), aResponse.get<std::string>("docType"));
    CPPUNIT_ASSERT_EQUAL(1, blockCount(aResponse));

    // Text aligns to the left by default, which goes without a field.
    const auto oText = cellTextAt(aResponse, 0, 0);
    CPPUNIT_ASSERT(oText);
    CPPUNIT_ASSERT(!oText->get_optional<std::string>("horizontal"));

    // A number aligns to the right by default.
    const auto oNumber = cellTextAt(aResponse, 1, 1);
    CPPUNIT_ASSERT(oNumber);
    CPPUNIT_ASSERT_EQUAL(std::string("right"), oNumber->get<std::string>("horizontal"));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorCellTextNamesItsFontByStyle)
{
    // Cells in the same font share one text style, sent once. Each text carries its own origin.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 0, 0, u"One"_ustr);
    setCellString(*pDocShell, 0, 1, u"Two"_ustr);

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");

    std::vector<std::string> aStyles;
    for (const auto& rText : cellTexts(aResponse))
    {
        const boost::property_tree::ptree& rPortion = rText.get_child("children").front().second;
        CPPUNIT_ASSERT_EQUAL(std::string("textSimplePortion"), rPortion.get<std::string>("type"));
        CPPUNIT_ASSERT_EQUAL(0, cellPosition(rPortion.get_child("x")).first);
        CPPUNIT_ASSERT(!rPortion.get_child_optional("familyname"));
        aStyles.push_back(rPortion.get<std::string>("style"));
    }
    CPPUNIT_ASSERT_EQUAL(size_t(2), aStyles.size());
    CPPUNIT_ASSERT_EQUAL(aStyles[0], aStyles[1]);

    const boost::property_tree::ptree& rStyles = aResponse.get_child("textStyles");
    CPPUNIT_ASSERT_EQUAL(size_t(1), rStyles.size());
    CPPUNIT_ASSERT(rStyles.get_child(aStyles[0]).get_optional<std::string>("familyname"));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorTextOverflowsIntoEmptyCells)
{
    // Long text in A1 overflows into the empty cells to its right and hides the grid lines there.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 0, 0,
                  u"A text that is much longer than the width of a single column"_ustr);

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    CPPUNIT_ASSERT(cellTextAt(aResponse, 0, 0));

    bool bHasCover = false;
    for (const auto& rRow : blockRows(aResponse))
    {
        if (rRow.get_child_optional("content.layers.gridCover"))
            bHasCover = true;
    }
    CPPUNIT_ASSERT(bHasCover);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorHeldBlockIsNotSentAgain)
{
    // A block the client holds in its current version is not sent again.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 0, 0, u"Hello"_ustr);

    const boost::property_tree::ptree aFirst
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    const sal_Int64 nVersion = blockVersion(aFirst);
    CPPUNIT_ASSERT(nVersion > 0);
    const std::string aEpoch = aFirst.get<std::string>("epoch");

    const boost::property_tree::ptree aSecond = requestVectorPrimitives(
        pModelObj, "partid=0&mode=-1&epoch=" + aEpoch + "&blocks=0:0:" + std::to_string(nVersion));
    CPPUNIT_ASSERT_EQUAL(0, blockCount(aSecond));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorDeltaCarriesChangedBlock)
{
    // A changed cell pushes a newer version of its block. A push with no change writes nothing.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 0, 0, u"Hello"_ustr);

    const boost::property_tree::ptree aFirst
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    const sal_Int64 nVersion = blockVersion(aFirst);

    setCellString(*pDocShell, 0, 0, u"World"_ustr);

    tools::JsonWriter aPushWriter;
    pModelObj->pushVectorPrimitivesDelta(aPushWriter, "0", sc::VectorContent::constCellPositionsMode);
    const boost::property_tree::ptree aDelta = parseJson(aPushWriter.finishAndGetAsOString());
    CPPUNIT_ASSERT_EQUAL(std::string("vectorprimitivesdelta"), aDelta.get<std::string>("type"));
    CPPUNIT_ASSERT_EQUAL(1, blockCount(aDelta));
    CPPUNIT_ASSERT(blockVersion(aDelta) > nVersion);

    tools::JsonWriter aSecondPushWriter;
    pModelObj->pushVectorPrimitivesDelta(aSecondPushWriter, "0", sc::VectorContent::constCellPositionsMode);
    CPPUNIT_ASSERT_EQUAL(tools::JsonWriter::EMPTY_DOCUMENT,
                         aSecondPushWriter.finishAndGetAsOString());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorDeltaCarriesOnlyTheChangedRow)
{
    // A changed cell pushes only its own row of the block.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 0, 0, u"First"_ustr);
    setCellString(*pDocShell, 0, 1, u"Second"_ustr);
    setCellString(*pDocShell, 0, 2, u"Third"_ustr);

    const boost::property_tree::ptree aFirst
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    CPPUNIT_ASSERT_EQUAL(size_t(3), blockRows(aFirst).size());

    setCellString(*pDocShell, 0, 1, u"Changed"_ustr);

    tools::JsonWriter aPushWriter;
    pModelObj->pushVectorPrimitivesDelta(aPushWriter, "0", sc::VectorContent::constCellPositionsMode);
    const boost::property_tree::ptree aDelta = parseJson(aPushWriter.finishAndGetAsOString());
    const std::vector<boost::property_tree::ptree> aRows = blockRows(aDelta);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aRows.size());
    CPPUNIT_ASSERT_EQUAL(1, aRows[0].get<int>("row"));
    CPPUNIT_ASSERT(cellTextAt(aDelta, 0, 1));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorEmptiedRowIsSentEmpty)
{
    // Clearing the last cell of a row pushes that row without content.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 0, 0, u"Hello"_ustr);

    requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");

    clearCell(*pDocShell, 0, 0);

    tools::JsonWriter aPushWriter;
    pModelObj->pushVectorPrimitivesDelta(aPushWriter, "0", sc::VectorContent::constCellPositionsMode);
    const boost::property_tree::ptree aDelta = parseJson(aPushWriter.finishAndGetAsOString());
    const std::vector<boost::property_tree::ptree> aRows = blockRows(aDelta);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aRows.size());
    CPPUNIT_ASSERT_EQUAL(0, aRows[0].get<int>("row"));
    CPPUNIT_ASSERT(!aRows[0].get_child_optional("content.layers"));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorNewSheetStartsNewEpoch)
{
    // Inserting a sheet changes which sheet a part id names, so a new epoch starts.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);

    const boost::property_tree::ptree aFirst
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    const std::string aEpoch = aFirst.get<std::string>("epoch");

    pDocShell->GetDocFunc().InsertTable(0, u"Inserted"_ustr, false, false);
    Scheduler::ProcessEventsToIdle();

    tools::JsonWriter aPushWriter;
    pModelObj->pushVectorPrimitivesDelta(aPushWriter, "0", sc::VectorContent::constCellPositionsMode);
    const boost::property_tree::ptree aDelta = parseJson(aPushWriter.finishAndGetAsOString());
    CPPUNIT_ASSERT(aEpoch != aDelta.get<std::string>("epoch"));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testPrimitiveTileStepsMatchTiles)
{
    // Fills and borders drawn from primitives match the painted tile, except at the edge pixels.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDocument = pDocShell->GetDocument();

    ScPatternAttr aPattern(rDocument.getCellAttributeHelper());
    aPattern.ItemSetPut(SvxBrushItem(COL_LIGHTRED, ATTR_BACKGROUND));
    SvxBoxItem aBox(ATTR_BORDER);
    const editeng::SvxBorderLine aLine(&COL_BLUE, 30);
    aBox.SetLine(&aLine, SvxBoxItemLine::TOP);
    aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);
    aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);
    aBox.SetLine(&aLine, SvxBoxItemLine::RIGHT);
    aPattern.ItemSetPut(aBox);
    rDocument.ApplyPatternAreaTab(1, 1, 2, 3, 0, aPattern);
    pDocShell->PostPaintGridAll();
    Scheduler::ProcessEventsToIdle();

    const std::vector<unsigned char> aTiles = paintTile(pModelObj);

    osl_setEnvironment(u"SC_PRIMITIVE_TILES"_ustr.pData, u"background,borders"_ustr.pData);
    const std::vector<unsigned char> aPrimitives = paintTile(pModelObj);
    osl_clearEnvironment(u"SC_PRIMITIVE_TILES"_ustr.pData);

    // Of the thousands of pixels in the area, only the few hundred at its edges may differ.
    CPPUNIT_ASSERT_LESS(1500, differingPixels(aTiles, aPrimitives));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorWrappedCurrencyWraps)
{
    // A number in a currency format wraps like text. Only a plain number format ignores the wrap.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDocument = pDocShell->GetDocument();
    rDocument.SetColWidth(0, 0, 800);
    setCellValue(*pDocShell, 0, 0, 123456789.0);

    const sal_uInt32 nCurrency
        = rDocument.GetFormatTable()->GetStandardFormat(SvNumFormatType::CURRENCY);
    ScPatternAttr aPattern(rDocument.getCellAttributeHelper());
    aPattern.ItemSetPut(ScLineBreakCell(true));
    aPattern.ItemSetPut(SfxUInt32Item(ATTR_VALUE_FORMAT, nCurrency));
    applyToCell(*pDocShell, 0, 0, aPattern);

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    const auto oText = cellTextAt(aResponse, 0, 0);
    CPPUNIT_ASSERT(oText);
    const std::set<std::string> aFlags = cellTextFlags(*oText);
    CPPUNIT_ASSERT(aFlags.count("wrap"));
    CPPUNIT_ASSERT(!aFlags.count("hashFill"));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorWrappedNumberShowsHashAndArrow)
{
    // A number in the general format ignores the wrap and shows "###" and an arrow when too wide.
    // The column is narrower than "1E+08", the shortest form of the number.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDocument = pDocShell->GetDocument();
    rDocument.SetColWidth(0, 0, 400);
    setCellValue(*pDocShell, 0, 0, 123456789.0);

    ScPatternAttr aPattern(rDocument.getCellAttributeHelper());
    aPattern.ItemSetPut(ScLineBreakCell(true));
    applyToCell(*pDocShell, 0, 0, aPattern);

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    const auto oText = cellTextAt(aResponse, 0, 0);
    CPPUNIT_ASSERT(oText);
    const std::set<std::string> aFlags = cellTextFlags(*oText);
    CPPUNIT_ASSERT(!aFlags.count("wrap"));
    CPPUNIT_ASSERT(aFlags.count("hashFill"));
    // A number aligns to the right, so it is cut off at the left.
    CPPUNIT_ASSERT(hasCellMark(aResponse, "clipLeft", 0, 0));
    CPPUNIT_ASSERT(!hasCellMark(aResponse, "clipRight", 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorShrunkTextGetsNoClipArrow)
{
    // Shrink-to-fit text shrinks to the width of its cell, so it is drawn whole with no arrow.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDocument = pDocShell->GetDocument();
    rDocument.SetColWidth(0, 0, 600);
    setCellString(*pDocShell, 0, 0,
                  u"A text that is far too long to shrink into one narrow column"_ustr);

    ScPatternAttr aPattern(rDocument.getCellAttributeHelper());
    aPattern.ItemSetPut(ScShrinkToFitCell(true));
    applyToCell(*pDocShell, 0, 0, aPattern);

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    const auto oText = cellTextAt(aResponse, 0, 0);
    CPPUNIT_ASSERT(oText);
    CPPUNIT_ASSERT(cellTextFlags(*oText).count("shrink"));
    CPPUNIT_ASSERT(!oText->get_child_optional("clip"));
    CPPUNIT_ASSERT(!hasCellMark(aResponse, "clipRight", 0, 0));
    CPPUNIT_ASSERT(!hasCellMark(aResponse, "clipLeft", 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorOverflowingTextStaysInTheBlockOfItsCell)
{
    // A block holds the text of its own cells. Text that overflows into the next block stays in the
    // block of its cell and is cut at the cell edge only when the next cell is filled.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    // Column P is the last column of the first block, column Q the first of the second.
    const SCCOL nLast = sc::VectorContent::constBlockColumns - 1;
    setCellString(*pDocShell, nLast, 0,
                  u"A text that is much longer than the width of a single column"_ustr);
    setCellString(*pDocShell, nLast + 1, 0, u"Next"_ustr);

    const auto oBlocked
        = cellTextAt(requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0"), nLast, 0);
    CPPUNIT_ASSERT(oBlocked);
    CPPUNIT_ASSERT(oBlocked->get_child_optional("clip"));
    const boost::property_tree::ptree aNextBlock
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:1");
    CPPUNIT_ASSERT(cellTextAt(aNextBlock, nLast + 1, 0));
    CPPUNIT_ASSERT(!cellTextAt(aNextBlock, nLast, 0));

    clearCell(*pDocShell, nLast + 1, 0);

    const auto oOverflowing
        = cellTextAt(requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0"), nLast, 0);
    CPPUNIT_ASSERT(oOverflowing);
    CPPUNIT_ASSERT(!oOverflowing->get_child_optional("clip"));
    CPPUNIT_ASSERT(
        !cellTextAt(requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:1"), nLast, 0));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorShapeEntryCarriesItsGeometry)
{
    // A shape carries the mapping of its unit square in twips from the top-left corner of the cell
    // under its own top-left corner, and travels in the block of that cell. A group member names
    // its group as its parent.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDocument = pDocShell->GetDocument();
    rDocument.InitDrawLayer(pDocShell);
    ScDrawLayer* pDrawLayer = rDocument.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    rtl::Reference<SdrRectObj> pShape
        = new SdrRectObj(*pDrawLayer, tools::Rectangle(Point(1000, 1000), Size(2000, 1000)));
    pPage->InsertObject(pShape.get());

    rtl::Reference<SdrObjGroup> pGroup = new SdrObjGroup(*pDrawLayer);
    rtl::Reference<SdrRectObj> pMember
        = new SdrRectObj(*pDrawLayer, tools::Rectangle(Point(5000, 5000), Size(1000, 1000)));
    pGroup->GetSubList()->InsertObject(pMember.get());
    pPage->InsertObject(pGroup.get());
    Scheduler::ProcessEventsToIdle();

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");

    const auto oShape = drawingEntry(aResponse, pShape->GetUniqueID());
    CPPUNIT_ASSERT(oShape);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), oShape->get<sal_Int64>("content.parent"));
    std::vector<double> aTransform;
    for (const auto& rValue : oShape->get_child("content.transform"))
        aTransform.push_back(rValue.second.get_value<double>());
    CPPUNIT_ASSERT_EQUAL(size_t(6), aTransform.size());
    // The rectangle is 2000 by 1000 at 1000, 1000, all in 1/100 mm. The anchor cell holds the
    // top-left corner, and the translation is measured from the start of that cell.
    const double fCorner = o3tl::convert(1000.0, o3tl::Length::mm100, o3tl::Length::twip);
    const SCCOL nAnchorColumn = oShape->get<SCCOL>("content.anchorColumn");
    const SCROW nAnchorRow = oShape->get<SCROW>("content.anchorRow");
    const SCTAB nTab = 0;
    const tools::Long nCellLeft
        = nAnchorColumn > 0 ? rDocument.GetColWidth(SCCOL(0), SCCOL(nAnchorColumn - 1), nTab) : 0;
    const tools::Long nCellTop
        = nAnchorRow > 0 ? rDocument.GetRowHeight(SCROW(0), SCROW(nAnchorRow - 1), nTab) : 0;
    CPPUNIT_ASSERT(nCellLeft <= fCorner);
    CPPUNIT_ASSERT(fCorner < nCellLeft + rDocument.GetColWidth(nAnchorColumn, nTab));
    CPPUNIT_ASSERT(nCellTop <= fCorner);
    CPPUNIT_ASSERT(fCorner < nCellTop + rDocument.GetRowHeight(nAnchorRow, nTab));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(o3tl::convert(2000.0, o3tl::Length::mm100, o3tl::Length::twip),
                                 aTransform[0], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(fCorner, aTransform[3], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(fCorner - nCellLeft, aTransform[4], 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(fCorner - nCellTop, aTransform[5], 1.0);

    const auto oMember = drawingEntry(aResponse, pMember->GetUniqueID());
    CPPUNIT_ASSERT(oMember);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(pGroup->GetUniqueID()),
                         oMember->get<sal_Int64>("content.parent"));
    CPPUNIT_ASSERT(drawingEntry(aResponse, pGroup->GetUniqueID()));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorRightToLeftSheetKeepsDocumentOrder)
{
    // A right-to-left sheet sends the same borders as a left-to-right one, in document order.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDocument = pDocShell->GetDocument();

    // Only the left edge of B2 has a line, so a mirrored cell would show it at another place.
    ScPatternAttr aPattern(rDocument.getCellAttributeHelper());
    SvxBoxItem aBox(ATTR_BORDER);
    const editeng::SvxBorderLine aLine(&COL_BLUE, 30);
    aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);
    aPattern.ItemSetPut(aBox);
    applyToCell(*pDocShell, 1, 1, aPattern);

    const boost::property_tree::ptree aLeftToRight
        = blockBorders(requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0"));
    CPPUNIT_ASSERT(!aLeftToRight.empty());

    rDocument.SetLayoutRTL(0, true);
    pDocShell->PostPaintGridAll();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(rDocument.IsLayoutRTL(0));

    const boost::property_tree::ptree aRightToLeft
        = blockBorders(requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0"));
    CPPUNIT_ASSERT_EQUAL(asJson(aLeftToRight), asJson(aRightToLeft));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorMergedCellsShowTheTextOfTheFirstCell)
{
    // A merged area shows the text of its first cell and hides the grid lines inside it.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 0, 0, u"Merged"_ustr);
    ScCellMergeOption aMerge(0, 0, 2, 2);
    aMerge.maTabs.insert(0);
    pDocShell->GetDocFunc().MergeCells(aMerge, false, true, true);
    Scheduler::ProcessEventsToIdle();

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    CPPUNIT_ASSERT(cellTextAt(aResponse, 0, 0));
    CPPUNIT_ASSERT_EQUAL(size_t(1), cellTexts(aResponse).size());

    const std::vector<std::vector<int>> aAreas = gridCoverAreas(aResponse);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aAreas.size());
    const std::vector<int> aExpected{ 0, 0, 2, 2 };
    CPPUNIT_ASSERT_EQUAL(aExpected.size(), aAreas[0].size());
    for (size_t nIndex = 0; nIndex < aExpected.size(); ++nIndex)
        CPPUNIT_ASSERT_EQUAL(aExpected[nIndex], aAreas[0][nIndex]);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorHiddenRowDrawsNoText)
{
    // The text of a cell in a hidden row is not sent. The rows around it keep their text.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 0, 0, u"Above"_ustr);
    setCellString(*pDocShell, 0, 1, u"Hidden"_ustr);
    setCellString(*pDocShell, 0, 2, u"Below"_ustr);
    pDocShell->GetDocument().SetRowHidden(1, 1, 0, true);
    pDocShell->PostPaintGridAll();
    Scheduler::ProcessEventsToIdle();

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    CPPUNIT_ASSERT(cellTextAt(aResponse, 0, 0));
    CPPUNIT_ASSERT(!cellTextAt(aResponse, 0, 1));
    CPPUNIT_ASSERT(cellTextAt(aResponse, 0, 2));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorRowHeightUpdatesMergedAreaFromBlockAbove)
{
    // A merged area that starts in the first block and reaches into the second moves its centred
    // text when a row of the second block gets taller. Text at the bottom would keep its position
    // in the last row of the area.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDocument = pDocShell->GetDocument();
    const SCROW nBlockRows = sc::VectorContent::constBlockRows;
    const SCROW nMergeStart = nBlockRows - 4;
    const SCROW nMergeEnd = nBlockRows + 4;
    setCellString(*pDocShell, 0, nMergeStart, u"Merged"_ustr);
    ScCellMergeOption aMerge(0, nMergeStart, 0, nMergeEnd);
    aMerge.maTabs.insert(0);
    pDocShell->GetDocFunc().MergeCells(aMerge, false, true, true);
    rDocument.ApplyAttr(0, nMergeStart, 0,
                        SvxVerJustifyItem(SvxCellVerJustify::Center, ATTR_VER_JUSTIFY));
    Scheduler::ProcessEventsToIdle();

    requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");

    // The text is centred in the merged area, so a taller row inside it moves the text.
    // The paint starts at the changed row, the way an adjusted row height reports it.
    const SCROW nChangedRow = nBlockRows + 2;
    rDocument.SetRowHeight(nChangedRow, 0, 2000);
    pDocShell->PostPaint(ScRange(0, nChangedRow, 0, rDocument.MaxCol(), rDocument.MaxRow(), 0),
                         PaintPartFlags::Grid | PaintPartFlags::Left);
    Scheduler::ProcessEventsToIdle();

    tools::JsonWriter aPushWriter;
    pModelObj->pushVectorPrimitivesDelta(aPushWriter, "0", sc::VectorContent::constCellPositionsMode);
    const boost::property_tree::ptree aDelta = parseJson(aPushWriter.finishAndGetAsOString());
    CPPUNIT_ASSERT(cellTextAt(aDelta, 0, nMergeStart));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorCellTextKeepsAutomaticFontColor)
{
    // Every view draws the same content, and an automatic font color is chosen against the
    // document color of a view. So the text keeps the color automatic and names the fill of its
    // cell, and each client chooses black or white for its own view.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDocument = pDocShell->GetDocument();
    setCellString(*pDocShell, 0, 0, u"Auto"_ustr);
    setCellString(*pDocShell, 0, 1, u"Dark"_ustr);
    setCellString(*pDocShell, 0, 2, u"Red"_ustr);
    ScPatternAttr aFilled(rDocument.getCellAttributeHelper());
    aFilled.ItemSetPut(SvxBrushItem(Color(0x000080), ATTR_BACKGROUND));
    rDocument.ApplyPatternAreaTab(0, 1, 0, 1, 0, aFilled);
    ScPatternAttr aRed(rDocument.getCellAttributeHelper());
    aRed.ItemSetPut(SvxColorItem(COL_LIGHTRED, ATTR_FONT_COLOR));
    rDocument.ApplyPatternAreaTab(0, 2, 0, 2, 0, aRed);

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    auto fontColorOf = [&aResponse](const boost::property_tree::ptree& rPortion)
    {
        return aResponse.get_child("textStyles")
            .get_child(rPortion.get<std::string>("style"))
            .get<std::string>("fontcolor");
    };
    for (const auto& rText : cellTexts(aResponse))
    {
        const boost::property_tree::ptree& rPortion = rText.get_child("children").front().second;
        const auto oBackdrop = rPortion.get_optional<std::string>("backdrop");
        switch (rText.get<int>("row"))
        {
            case 0:
                CPPUNIT_ASSERT_EQUAL(std::string("auto"), fontColorOf(rPortion));
                CPPUNIT_ASSERT(!oBackdrop);
                break;
            case 1:
                CPPUNIT_ASSERT_EQUAL(std::string("auto"), fontColorOf(rPortion));
                CPPUNIT_ASSERT_EQUAL(std::string("#000080"), oBackdrop.value_or(""));
                break;
            case 2:
                CPPUNIT_ASSERT_EQUAL(std::string("#ff0000"), fontColorOf(rPortion));
                break;
        }
    }
    CPPUNIT_ASSERT_EQUAL(size_t(3), cellTexts(aResponse).size());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorCellPositionsFill)
{
    // The fill of B2 runs from the start of column B to the start of column C, and from the start
    // of row 2 to the start of row 3.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDocument = pDocShell->GetDocument();
    ScPatternAttr aPattern(rDocument.getCellAttributeHelper());
    aPattern.ItemSetPut(SvxBrushItem(COL_LIGHTRED, ATTR_BACKGROUND));
    rDocument.ApplyPatternAreaTab(1, 1, 1, 1, 0, aPattern);
    pDocShell->PostPaintGridAll();
    Scheduler::ProcessEventsToIdle();

    const boost::property_tree::ptree aResponse
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    std::optional<boost::property_tree::ptree> oFill;
    for (const auto& rRow : blockRows(aResponse))
    {
        if (rRow.get<int>("row") != 1)
            continue;
        if (auto oBackground = rRow.get_child_optional("content.layers.background"))
            oFill = oBackground->begin()->second;
    }
    CPPUNIT_ASSERT(oFill);
    CPPUNIT_ASSERT_EQUAL(std::string("M1:0,1:0L2:0,1:0L2:0,2:0L1:0,2:0Z"),
                         oFill->get<std::string>("path"));

    // The bounds are positions too.
    const auto& rBounds = oFill->get_child("bounds");
    auto aEdge = rBounds.begin();
    const std::pair<int, double> aLeft = cellPosition(aEdge->second);
    CPPUNIT_ASSERT_EQUAL(1, aLeft.first);
    CPPUNIT_ASSERT_EQUAL(0.0, aLeft.second);
    ++aEdge;
    const std::pair<int, double> aTop = cellPosition(aEdge->second);
    CPPUNIT_ASSERT_EQUAL(1, aTop.first);
    CPPUNIT_ASSERT_EQUAL(0.0, aTop.second);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorCellPositionsTextOrigin)
{
    // The text of D2 starts in column D, a margin from its left edge, and on its baseline in row 2.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 3, 1, u"Hello"_ustr);
    Scheduler::ProcessEventsToIdle();

    const auto oText = cellTextAt(
        requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0"), 3, 1);
    CPPUNIT_ASSERT(oText);
    const boost::property_tree::ptree& rPortion = oText->get_child("children").begin()->second;
    const auto [nColumn, fX] = cellPosition(rPortion.get_child("x"));
    const auto [nRow, fY] = cellPosition(rPortion.get_child("y"));
    CPPUNIT_ASSERT_EQUAL(3, nColumn);
    CPPUNIT_ASSERT_EQUAL(1, nRow);
    const ScDocument& rDocument = pDocShell->GetDocument();
    CPPUNIT_ASSERT_GREATER(0.0, fX);
    CPPUNIT_ASSERT_LESS(double(rDocument.GetColWidth(3, 0)), fX);
    CPPUNIT_ASSERT_GREATER(0.0, fY);
    CPPUNIT_ASSERT_LESS(double(rDocument.GetRowHeight(1, 0)), fY);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorCellPositionsClipOutsideTheSheet)
{
    // A text cut off at its right neighbor is clipped only across, so its clip reaches far above
    // and below the sheet. Those edges are positions in the first and the last row too, so the
    // browser places every edge from the cells.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    setCellString(*pDocShell, 0, 2, u"A text much longer than its cell"_ustr);
    setCellString(*pDocShell, 1, 2, u"Next"_ustr);
    Scheduler::ProcessEventsToIdle();

    const auto oText = cellTextAt(
        requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0"), 0, 2);
    CPPUNIT_ASSERT(oText);
    // Across, the clip ends in the cell, before the room for the arrow that marks the cut.
    const boost::property_tree::ptree& rClip = oText->get_child("clip");
    CPPUNIT_ASSERT_EQUAL(0, cellPosition(rClip.get_child("right")).first);
    const auto [nTop, fTop] = cellPosition(rClip.get_child("y"));
    CPPUNIT_ASSERT_EQUAL(0, nTop);
    CPPUNIT_ASSERT(fTop < 0);
    const auto [nBottom, fBottom] = cellPosition(rClip.get_child("bottom"));
    CPPUNIT_ASSERT_EQUAL(int(pDocShell->GetDocument().MaxRow()), nBottom);
    CPPUNIT_ASSERT(fBottom > 0);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorCompletedCellEditIsPushedAgain)
{
    // A text typed into a cell is completed from its column: the first character shows the whole
    // text with the completion selected, and the next character replaces the selection and is
    // completed again. The edit then reads as before and only the selection moved. A client shows
    // what it typed until the engine answers, so the edit is pushed again all the same.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    setCellString(*pDocShell, 1, 3, u"Teszt asztal v"_ustr);
    requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");

    typeCharsInCell("T", 1, 5, pView, pModelObj, /*bInEdit*/ false, /*bCommit*/ false);
    ScViewData& rViewData = pView->GetViewData();
    EditView* pEditView = rViewData.GetEditView(rViewData.GetEditActivePart());
    CPPUNIT_ASSERT(pEditView);
    CPPUNIT_ASSERT_EQUAL(u"Teszt asztal v"_ustr, pEditView->getEditEngine().GetText());
    {
        tools::JsonWriter aPushWriter;
        pModelObj->pushVectorPrimitivesDelta(aPushWriter, "0",
                                             sc::VectorContent::constCellPositionsMode);
        CPPUNIT_ASSERT(
            parseJson(aPushWriter.finishAndGetAsOString()).get_child_optional("textEdits"));
    }

    typeCharsInCell("e", 1, 5, pView, pModelObj, /*bInEdit*/ true, /*bCommit*/ false);
    CPPUNIT_ASSERT_EQUAL(u"Teszt asztal v"_ustr, pEditView->getEditEngine().GetText());
    tools::JsonWriter aPushWriter;
    pModelObj->pushVectorPrimitivesDelta(aPushWriter, "0",
                                         sc::VectorContent::constCellPositionsMode);
    const OString aPush = aPushWriter.finishAndGetAsOString();
    CPPUNIT_ASSERT(aPush != tools::JsonWriter::EMPTY_DOCUMENT);
    CPPUNIT_ASSERT(parseJson(aPush).get_child_optional("textEdits"));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorCellEditGrowsOutsideASmallWindow)
{
    // Painting a tile makes the window of the view as large as the document, which the cell editing
    // of a kit view counts on. A view served as vectors paints no tiles, so its window kept a small
    // size, and an edit in a cell outside it had no room: it wrapped at the width of its cell and
    // did not grow into the empty cells after it.
    comphelper::COKit::setCompatFlag(comphelper::COKit::Compat::scPrintTwipsMsgs);
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");

    // Moving the cursor can extend the tiled area, which sizes the window too, so the window is
    // made small after it, as a session that never painted a tile leaves it. Then the zoom
    // changes: 256 pixels show 1280 twips, a zoom of 300 %.
    pView->SetCursor(15, 7);
    Scheduler::ProcessEventsToIdle();
    ScViewData& rViewData = pView->GetViewData();
    rViewData.GetActiveWin()->SetOutputSizePixel(Size(751, 442));
    pModelObj->setClientZoom(256, 256, 1280, 1280);

    typeCharsInCell("A text much longer than its cell", 15, 7, pView, pModelObj, /*bInEdit*/ true,
                    /*bCommit*/ false);
    Scheduler::ProcessEventsToIdle();

    EditView* pEditView = rViewData.GetEditView(rViewData.GetEditActivePart());
    CPPUNIT_ASSERT(pEditView);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pEditView->getEditEngine().GetLineCount(0));
    CPPUNIT_ASSERT(rViewData.GetEditEndCol() > 15);
}

namespace
{
/// The keys of the blocks that a response names stale.
std::set<std::string> staleBlocks(const boost::property_tree::ptree& rResponse)
{
    std::set<std::string> aKeys;
    if (auto oStale = rResponse.get_child_optional("stale"))
    {
        for (const auto& rKey : *oStale)
            aKeys.insert(rKey.second.get_value<std::string>());
    }
    return aKeys;
}

/// Ask for the blocks of the first column of blocks, from block row 0 to nBlockRows - 1, in cell
/// positions, and give the epoch of the answer.
std::string requestBlockColumn(ScModelObj* pModelObj, int nBlockRows)
{
    std::string aBlocks;
    for (int nBlockRow = 0; nBlockRow < nBlockRows; ++nBlockRow)
        aBlocks += (nBlockRow ? "," : "") + std::to_string(nBlockRow) + ":0";
    const boost::property_tree::ptree aAnswer
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=" + aBlocks);
    CPPUNIT_ASSERT_EQUAL(nBlockRows, blockCount(aAnswer));
    return aAnswer.get<std::string>("epoch");
}

boost::property_tree::ptree pushCellPositions(ScModelObj* pModelObj)
{
    tools::JsonWriter aWriter;
    pModelObj->pushVectorPrimitivesDelta(aWriter, "0", sc::VectorContent::constCellPositionsMode);
    const OString aJson = aWriter.finishAndGetAsOString();
    if (aJson == tools::JsonWriter::EMPTY_DOCUMENT)
        return {};
    return parseJson(aJson);
}
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorManyChangedBlocksAreNamedStale)
{
    // A column that changes size may change every held block. They are not built again at once:
    // the push names them stale, and a block is built when a client asks for it.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    const int nBlockRows = int(sc::VectorContent::constEagerBlocks) + 4;
    for (int nBlockRow = 0; nBlockRow < nBlockRows; ++nBlockRow)
        setCellString(*pDocShell, 0, nBlockRow * sc::VectorContent::constBlockRows,
                      u"Text"_ustr);
    const std::string aEpoch = requestBlockColumn(pModelObj, nBlockRows);

    setColumnWidth(*pDocShell, 0, 3000);

    const boost::property_tree::ptree aPush = pushCellPositions(pModelObj);
    CPPUNIT_ASSERT_EQUAL(0, blockCount(aPush));
    const std::set<std::string> aStale = staleBlocks(aPush);
    CPPUNIT_ASSERT_EQUAL(size_t(nBlockRows), aStale.size());
    CPPUNIT_ASSERT(aStale.count("3:0"));

    // Asked for, the block is built and comes with the answer, which names no stale blocks: the
    // push did, once, and the others are still stale.
    const boost::property_tree::ptree aAnswer = requestVectorPrimitives(
        pModelObj, "partid=0&mode=-1&epoch=" + aEpoch + "&blocks=3:0:1");
    CPPUNIT_ASSERT_EQUAL(1, blockCount(aAnswer));
    CPPUNIT_ASSERT(staleBlocks(aAnswer).empty());

    // The blocks were named once, so the next push has nothing to say.
    CPPUNIT_ASSERT(pushCellPositions(pModelObj).empty());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorFewChangedBlocksArePushedAfterStaleOnes)
{
    // Blocks still stale from an earlier change do not count against a later one, so a change to
    // one cell is pushed at once.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    const int nBlockRows = int(sc::VectorContent::constEagerBlocks) + 4;
    for (int nBlockRow = 0; nBlockRow < nBlockRows; ++nBlockRow)
        setCellString(*pDocShell, 0, nBlockRow * sc::VectorContent::constBlockRows,
                      u"Text"_ustr);
    requestBlockColumn(pModelObj, nBlockRows);
    setColumnWidth(*pDocShell, 0, 3000);
    CPPUNIT_ASSERT_EQUAL(size_t(nBlockRows), staleBlocks(pushCellPositions(pModelObj)).size());

    // The client asks for block 2:0, which is built again. A cell typed in it afterwards is
    // pushed at once, although the other blocks are still stale.
    requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=2:0");
    setCellString(*pDocShell, 0, 2 * sc::VectorContent::constBlockRows + 1, u"Typed"_ustr);

    const boost::property_tree::ptree aPush = pushCellPositions(pModelObj);
    CPPUNIT_ASSERT_EQUAL(1, blockCount(aPush));
    CPPUNIT_ASSERT(cellTextAt(aPush, 0, 2 * sc::VectorContent::constBlockRows + 1));
    // The blocks still stale were named in the push before, so this one names none.
    CPPUNIT_ASSERT(staleBlocks(aPush).empty());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorSheetSettingsChangeLeavesBlocksCurrent)
{
    // Renaming a sheet paints the extras of every sheet. No block draws them, so the held blocks
    // are not named stale: the push carries the sheet settings.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    const int nBlockRows = int(sc::VectorContent::constEagerBlocks) + 4;
    for (int nBlockRow = 0; nBlockRow < nBlockRows; ++nBlockRow)
        setCellString(*pDocShell, 0, nBlockRow * sc::VectorContent::constBlockRows,
                      u"Text"_ustr);
    requestBlockColumn(pModelObj, nBlockRows);

    CPPUNIT_ASSERT(pDocShell->GetDocFunc().RenameTable(0, u"Renamed"_ustr, false, true));

    const boost::property_tree::ptree aPush = pushCellPositions(pModelObj);
    CPPUNIT_ASSERT(staleBlocks(aPush).empty());
    CPPUNIT_ASSERT_EQUAL(0, blockCount(aPush));
    CPPUNIT_ASSERT_EQUAL(std::string("Renamed"), aPush.get<std::string>("sheet.name"));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorHiddenSheetKeepsEpoch)
{
    // Hiding a sheet renumbers no part id, so the clients keep what they hold.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    pDocShell->GetDocFunc().InsertTable(1, u"Second"_ustr, false, true);
    const boost::property_tree::ptree aFirst
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    const std::string aEpoch = aFirst.get<std::string>("epoch");

    cpo::uno::Reference<com::sun::star::container::XIndexAccess> xSheets(
        pModelObj->getSheets(), cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<com::sun::star::beans::XPropertySet> xSecond(xSheets->getByIndex(1),
                                                                     cpo::uno::UNO_QUERY_THROW);
    xSecond->setPropertyValue(u"IsVisible"_ustr, cpo::uno::Any(false));
    Scheduler::ProcessEventsToIdle();

    const boost::property_tree::ptree aAnswer = requestVectorPrimitives(
        pModelObj, "partid=0&mode=-1&epoch=" + aEpoch + "&blocks=0:0");
    CPPUNIT_ASSERT_EQUAL(aEpoch, aAnswer.get<std::string>("epoch"));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorChangedBlocksShownByAViewAreBuilt)
{
    // Of many changed blocks, the ones a view shows are built and pushed at once, so the view
    // does not have to ask for them. The others are named stale.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    const ScDocument& rDocument = pDocShell->GetDocument();
    const int nBlockRows = int(sc::VectorContent::constEagerBlocks) + 4;
    for (int nBlockRow = 0; nBlockRow < nBlockRows; ++nBlockRow)
        setCellString(*pDocShell, 0, nBlockRow * sc::VectorContent::constBlockRows,
                      u"Text"_ustr);
    requestBlockColumn(pModelObj, nBlockRows);

    // The current view shows the first column of one block row.
    auto showBlockRow = [&](int nBlockRow)
    {
        const SCROW nFirstRow = nBlockRow * sc::VectorContent::constBlockRows;
        const tools::Long nTop = nFirstRow ? rDocument.GetRowHeight(0, nFirstRow - 1, 0) : 0;
        const tools::Long nHeight
            = rDocument.GetRowHeight(nFirstRow, nFirstRow + sc::VectorContent::constBlockRows - 1, 0);
        pModelObj->setClientVisibleArea(
            tools::Rectangle(Point(0, nTop), Size(rDocument.GetColWidth(0, 0), nHeight)));
    };
    showBlockRow(3);
    KitHelper::createView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<com::sun::star::beans::PropertyValue>());
    showBlockRow(7);

    // A column too narrow for the text makes it overflow, which changes the text of every block.
    setColumnWidth(*pDocShell, 0, 300);

    const boost::property_tree::ptree aPush = pushCellPositions(pModelObj);
    CPPUNIT_ASSERT_EQUAL(2, blockCount(aPush));
    CPPUNIT_ASSERT(cellTextAt(aPush, 0, 3 * sc::VectorContent::constBlockRows));
    CPPUNIT_ASSERT(cellTextAt(aPush, 0, 7 * sc::VectorContent::constBlockRows));
    const std::set<std::string> aStale = staleBlocks(aPush);
    CPPUNIT_ASSERT_EQUAL(size_t(nBlockRows - 2), aStale.size());
    CPPUNIT_ASSERT(!aStale.count("3:0"));
    CPPUNIT_ASSERT(!aStale.count("7:0"));

    KitHelper::destroyView(KitHelper::getCurrentView());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorBlockWithDroppedContentKeepsItsVersion)
{
    // The engine keeps the JSON of a limited number of blocks of a sheet. A block whose JSON was
    // dropped is built again when a client asks for it, and the parts that did not change keep
    // their versions, so a client holding the block is not sent it again.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    const int nBlockRows = int(sc::VectorContent::constHeldContentBlocks) + 2;
    for (int nBlockRow = 0; nBlockRow < nBlockRows; ++nBlockRow)
        setCellString(*pDocShell, 0, nBlockRow * sc::VectorContent::constBlockRows,
                      u"Text"_ustr);

    const boost::property_tree::ptree aFirst
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    const std::string aEpoch = aFirst.get<std::string>("epoch");
    const sal_Int64 nVersion = blockVersion(aFirst);
    CPPUNIT_ASSERT(nVersion > 0);

    // Enough other blocks are asked for to drop the content of the first one.
    std::string aBlocks;
    for (int nBlockRow = 1; nBlockRow < nBlockRows; ++nBlockRow)
        aBlocks += (nBlockRow > 1 ? "," : "") + std::to_string(nBlockRow) + ":0";
    CPPUNIT_ASSERT_EQUAL(nBlockRows - 1,
                         blockCount(requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks="
                                                                           + aBlocks)));

    // Held in its version, the first block is not sent again, and asked for whole it comes in
    // the same version with its text.
    const boost::property_tree::ptree aHeld = requestVectorPrimitives(
        pModelObj, "partid=0&mode=-1&epoch=" + aEpoch + "&blocks=0:0:" + std::to_string(nVersion));
    CPPUNIT_ASSERT_EQUAL(0, blockCount(aHeld));
    const boost::property_tree::ptree aWhole
        = requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");
    CPPUNIT_ASSERT_EQUAL(1, blockCount(aWhole));
    CPPUNIT_ASSERT_EQUAL(nVersion, blockVersion(aWhole));
    CPPUNIT_ASSERT(cellTextAt(aWhole, 0, 0));
}

namespace
{
/// The shifts of rows or columns that a push names, each as "axis at by first,last".
std::vector<std::string> shiftsOf(const boost::property_tree::ptree& rResponse)
{
    std::vector<std::string> aShifts;
    if (auto oShifts = rResponse.get_child_optional("shifts"))
    {
        for (const auto& rShift : *oShifts)
        {
            std::string aAcross;
            for (const auto& rIndex : rShift.second.get_child("across"))
                aAcross += (aAcross.empty() ? "" : ",") + rIndex.second.get_value<std::string>();
            aShifts.push_back(rShift.second.get<std::string>("axis") + " "
                              + rShift.second.get<std::string>("at") + " "
                              + rShift.second.get<std::string>("by") + " " + aAcross);
        }
    }
    return aShifts;
}
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVectorInsertedAndRemovedRowsArePushedAsShifts)
{
    // Rows inserted or removed move the cells after them. The push says so once, so a client
    // shows a stale block moved the same way until the block is built again.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    const ScDocument& rDocument = pDocShell->GetDocument();
    const std::string aAllColumns = "0," + std::to_string(rDocument.MaxCol());
    setCellString(*pDocShell, 0, 0, u"Text"_ustr);
    requestVectorPrimitives(pModelObj, "partid=0&mode=-1&blocks=0:0");

    CPPUNIT_ASSERT(pDocShell->GetDocFunc().InsertCells(
        ScRange(0, 10, 0, rDocument.MaxCol(), 12, 0), nullptr, INS_INSROWS_BEFORE, false, true));
    std::vector<std::string> aShifts = shiftsOf(pushCellPositions(pModelObj));
    CPPUNIT_ASSERT_EQUAL(size_t(1), aShifts.size());
    CPPUNIT_ASSERT_EQUAL("rows 10 3 " + aAllColumns, aShifts[0]);

    // Removed rows lie before the first row that moved, by the distance it moved.
    CPPUNIT_ASSERT(pDocShell->GetDocFunc().DeleteCells(ScRange(0, 5, 0, rDocument.MaxCol(), 6, 0),
                                                       nullptr, DelCellCmd::Rows, true));
    aShifts = shiftsOf(pushCellPositions(pModelObj));
    CPPUNIT_ASSERT_EQUAL(size_t(1), aShifts.size());
    CPPUNIT_ASSERT_EQUAL("rows 7 -2 " + aAllColumns, aShifts[0]);

    CPPUNIT_ASSERT(shiftsOf(pushCellPositions(pModelObj)).empty());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
