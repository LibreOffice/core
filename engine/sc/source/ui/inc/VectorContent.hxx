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

#pragma once

#include <address.hxx>
#include <cpo/uno/Reference.hxx>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <drawinglayer/processor2d/VectorResourceCache.hxx>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <svl/lstner.hxx>
#include <svx/VectorPart.hxx>
#include <tools/gen.hxx>
#include <tools/long.hxx>

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

class ScDocShell;
class ScDocument;
class ScTabViewShell;
class ScRange;
class SfxHint;
class SdrObject;

namespace tools
{
class JsonWriter;
}

namespace sc
{
class ChartModifyListener;
class SheetListener;
struct CellPrimitiveLayers;

/** The content of the sheets of one document as JSON primitives, for clients that draw it
    themselves.

    The part is the sheet index. The cells are cut into blocks of a fixed number of rows and
    columns, named by block row and block column. The cells are laid out in the exact geometry of
    the document, and each position of their primitives is written relative to the cells, as
    [column, offset] or [row, offset] (see CellPositionJsonProcessor), so the content stays right
    when a column or a row changes size. A block has parts: the borders of the whole block, and the
    other layers of each row. A drawing object is named by its unique id and anchored to a cell, in
    twips from the top-left corner of that cell. It travels with the block that holds its anchor
    cell, and the sheet names the blocks whose objects paint over which cells.

    The mode of a request names nothing about the content: a sheet is held once for each mode a
    client asks for, so a client gets its answers and pushes in the mode it asked in.

    Every block part and drawing object carries the version of its last change, and a block the
    newest version of its parts. Versions count up from zero per sheet in the document epoch. A
    change that leaves the JSON as it was keeps the version, so a client gets only changed parts.
 */
class VectorContent final : public SfxListener
{
public:
    /// The number of rows in a block of cells.
    static constexpr SCROW constBlockRows = 64;
    /// The number of columns in a block of cells.
    static constexpr SCCOL constBlockColumns = 16;
    /// The mode the browser asks in.
    static constexpr sal_Int32 constCellPositionsMode = -1;
    /// The most blocks that changes are built again for at once. When more of the held blocks may
    /// have changed, as after a column or a row changed size, only the ones a view shows are
    /// built, and the others are named stale, so each is built again when a client asks for it.
    static constexpr size_t constEagerBlocks = 16;
    /// The most blocks of a sheet whose JSON is kept. The JSON of the blocks used longest ago is
    /// dropped, and such a block is built again when a client asks for it or a push carries it,
    /// with the versions of its parts kept where they did not change.
    static constexpr size_t constHeldContentBlocks = 128;

    explicit VectorContent(ScDocShell& rDocShell);
    virtual ~VectorContent() override;

    /// Answer the commands svx::vector::isPartCommand names.
    void getCommandValues(tools::JsonWriter& rWriter, std::string_view rCommand);

    /// Write the changes of a sheet since it was last pushed, or nothing when it did not change.
    void pushDelta(tools::JsonWriter& rWriter, std::string_view rPartId, sal_Int32 nMode);

    /// Take a hint about painted ranges, finished changes, or inserted, removed, moved sheets.
    void documentHint(const SfxHint& rHint);

    /// The cells of the range got new results, or get them when they are next read.
    void cellsChanged(const ScRange& rRange);

    /// The text of a cell or a shape that is edited in place on the sheet changed.
    void textEditChanged(SCTAB nTab);

    /// The chart with this model, on the sheet, changed.
    void chartChanged(SCTAB nTab, const cpo::uno::Reference<cpo::uno::XInterface>& rChart);

    /// The block that holds a cell.
    static std::pair<sal_Int32, sal_Int32> blockOf(SCCOL nColumn, SCROW nRow)
    {
        return { nRow / constBlockRows, nColumn / constBlockColumns };
    }

    virtual void Notify(SfxBroadcaster& rBroadcaster, const SfxHint& rHint) override;

private:
    using BlockKey = std::pair<sal_Int32, sal_Int32>;

    struct EntryState
    {
        sal_uInt64 mnVersion = 0;
        /// The JSON of the content, which is compared to see whether the entry changed. Empty
        /// for a block whose content was dropped.
        OString maContent;
        /// The hash of the content, 0 for no content, which decides whether the entry changed
        /// after its content was dropped.
        sal_uInt64 mnHash = 0;
        /// The ids of the text styles that the content names.
        std::vector<sal_Int32> maTextStyles;
    };

    struct BlockState
    {
        /// The newest version of the parts of the block, its objects included.
        sal_uInt64 mnVersion = 0;
        /// The version the block was first written at. No earlier version of the block exists.
        sal_uInt64 mnCreatedVersion = 0;
        /// Something in the block may have changed since its content was written.
        bool mbDirty = false;
        /// The rows that may have changed, both included, when not every row of the block may
        /// have. The borders of the whole block are looked at again either way.
        std::optional<std::pair<SCROW, SCROW>> moDirtyRows;
        /// A push named the block stale since it was marked, so the clients know to ask for it.
        bool mbStaleSent = false;
        /// The JSON of the parts is held. It is dropped from the blocks used longest ago.
        bool mbContentHeld = false;
        /// The count of the sheet when the block was last built or asked for.
        sal_uInt64 mnLastUse = 0;
        /// The JSON of the range the cells of the block cover.
        OString maBounds;
        /// The borders of the whole block. An empty content means there are none.
        EntryState maBorders;
        /// The other layers of each block row, from the first on. A row that draws nothing is
        /// empty.
        std::vector<EntryState> maRows;
    };

    /// A block as it was just written, in the parts of BlockState. The parts carry no version.
    struct BlockContent
    {
        OString maBounds;
        EntryState maBorders;
        std::vector<EntryState> maRows;
        /// The index of the first row that was written, and one past the last. The other rows of
        /// maRows are empty and say nothing about the block.
        std::pair<size_t, size_t> maBuiltRows{ 0, 0 };
    };

    /// Rows or columns inserted or removed, which moved the cells after them.
    struct Shift
    {
        bool mbRows;
        /// The first row or column moved, and how far. Removed ones lie before it, by a negative
        /// distance.
        SCCOLROW mnAt;
        SCCOLROW mnBy;
        /// The columns, for rows, or the rows, for columns, across which the cells moved.
        SCCOLROW mnAcrossFirst;
        SCCOLROW mnAcrossLast;
    };

    /// A drawing object as it was last written.
    struct ObjectState
    {
        EntryState maEntry;
        /// The cell the object is anchored to, which its content is written from.
        ScAddress maAnchor;
        /// The block holding the anchor cell, which the object travels with.
        BlockKey maBlock;
        /// The cells the object paints over.
        ScRange maCells;
    };

    struct SheetState
    {
        SCTAB mnTab = 0;
        sal_Int32 mnMode = 0;
        /// The last version handed out on the sheet, the version the sheet was last pushed at,
        /// and the version at which the drawing objects or their order changed last.
        svx::vector::PartVersions maVersions;

        std::map<BlockKey, BlockState> maBlocks;
        /// Counts the uses of the blocks, so the ones used longest ago are known.
        sal_uInt64 mnUseCount = 0;

        /// Every drawing object may have changed.
        bool mbObjectsDirty = true;
        /// The drawing objects that may have changed, by id, when not every one may have.
        std::set<sal_uInt64> maDirtyObjects;
        std::unordered_map<sal_uInt64, ObjectState> maObjects;
        /// The version at which an object left a block, by block and object id. An object that
        /// moved to another block left the one it was in.
        std::map<std::pair<BlockKey, sal_uInt64>, sal_uInt64> maRemovedObjects;
        std::vector<sal_uInt64> maObjectOrder;
        /// The blocks that hold objects, each with the cells its objects paint over.
        EntryState maObjectBlocks;

        EntryState maSheet;
        /// The text edits as they were last pushed.
        OString maLastTextEdits;
        /// A text edit changed since the last push. The push goes out even when the edits read
        /// the same, because a client shows what it typed until the engine answers.
        bool mbTextEditChanged = false;
        /// The rows and columns moved since the last push, which tells the clients, so they show
        /// a stale block moved the same way until it is built again.
        std::vector<Shift> maShifts;
    };

    /// A block asked for, with the version the client holds when it holds one.
    struct BlockRequest
    {
        BlockKey maKey;
        std::optional<sal_uInt64> moHeldVersion;
    };

    /// Answer a request for the primitives of a sheet.
    void writeRequestedSheet(tools::JsonWriter& rWriter, const svx::vector::PartRequest& rRequest);

    void writeVectorPrimitives(tools::JsonWriter& rWriter, SheetState& rState,
                               const std::vector<BlockRequest>& rBlocks, sal_Int64 nSinceVersion,
                               bool bPush, std::optional<sal_Int64> oRequestId = std::nullopt);

    void listenToDrawLayer();
    void listenToSheet(SCTAB nTab);
    SheetState& sheetState(SCTAB nTab, sal_Int32 nMode);
    /// Drop every sheet state, start a new epoch and tell the views for each held sheet and mode.
    void resetSheets();
    /// Mark the rows of the range in every held block of its block rows.
    void markRangeDirty(const ScRange& rRange);
    static void markRowsDirty(BlockState& rBlock, SCROW nStartRow, SCROW nEndRow, bool bWholeBlock);
    void markObjectDirty(SCTAB nTab, sal_uInt64 nId);
    /// The states of a sheet, one for each mode it was asked in.
    std::vector<SheetState*> statesOf(SCTAB nTab);
    void notifyViews(SCTAB nTab);
    /// Tell the views of the sheets whose cells changed since they were last told.
    void notifyChangedSheets();

    /// Build the blocks that may have changed since the last push again, unless there are more
    /// of them than constEagerBlocks: then only the blocks a view shows are built, and the others
    /// wait for a client to ask for them, as the blocks named stale before do.
    void resolveDirtyBlocks(SheetState& rState);
    /// A range of blocks, both ends included.
    struct BlockRange
    {
        sal_Int32 mnFirstRow;
        sal_Int32 mnLastRow;
        sal_Int32 mnFirstColumn;
        sal_Int32 mnLastColumn;

        bool holds(const BlockKey& rKey) const
        {
            return rKey.first >= mnFirstRow && rKey.first <= mnLastRow
                   && rKey.second >= mnFirstColumn && rKey.second <= mnLastColumn;
        }
    };
    /// The blocks of the sheet that the views show, from the visible areas their clients told.
    std::vector<BlockRange> shownBlocks(SCTAB nTab) const;
    /// The row at a position down the sheet, in twips, or the last row for a position past its
    /// end.
    static SCROW rowAt(const ScDocument& rDocument, SCTAB nTab, tools::Long nTwips);
    /// The column at a position across the sheet, in twips, or the last column for a position
    /// past its end.
    static SCCOL columnAt(const ScDocument& rDocument, SCTAB nTab, tools::Long nTwips);
    /// The number of the sheet, which is also its part id, or nothing when the id names no sheet.
    std::optional<SCTAB> sheetOfPartId(std::u16string_view aPartId) const;
    /// The cell an object not inside a group is anchored to. An object anchored to the page takes
    /// the cell under its top-left corner, the way the office anchors it when asked to.
    static ScAddress anchorCellOf(const ScDocument& rDocument, SCTAB nTab, SdrObject& rObject);
    /// The top-left corner of a cell in twips from the top-left of the sheet. On a sheet laid out
    /// right to left the sheet is mirrored, so the corner is left of the origin.
    static Point cellStartTwips(const ScDocument& rDocument, SCTAB nTab, const ScAddress& rCell);
    /// The cells a box in twips paints over, or the anchor cell alone for an empty box.
    static ScRange cellsOf(const ScDocument& rDocument, SCTAB nTab,
                           const tools::Rectangle& rBoxTwips, const ScAddress& rAnchor);
    /// The JSON of the layers of one block part, with their paint range and text styles, and the
    /// positions relative to the cells from the origin cell on. The JSON is empty when the layers
    /// draw nothing.
    void writeLayers(const CellPrimitiveLayers& rLayers, SCTAB nTab, SCCOL nOriginColumn,
                     SCROW nOriginRow, OString& rContent, std::vector<sal_Int32>& rUsedTextStyles);
    /// Name the held blocks that may have changed and were not built again, and were not named
    /// in a push before.
    static void writeStaleBlocks(tools::JsonWriter& rWriter, const SheetState& rState);
    /// Build the block again where it may have changed, or whole where its content was dropped,
    /// and give the changed parts a new version.
    void resolveBlock(SheetState& rState, const BlockKey& rKey);
    /// Drop the JSON of the blocks used longest ago beyond constHeldContentBlocks.
    static void dropOldContent(SheetState& rState);
    static sal_uInt64 contentHash(std::string_view rContent);
    /// Write the borders of the block and the given rows, both included, or every row.
    BlockContent writeBlockContent(const SheetState& rState, const BlockKey& rKey,
                                   const std::optional<std::pair<SCROW, SCROW>>& roRows);
    void resolveObjects(SheetState& rState);
    /// Write the object from its anchor cell, and record the cells it paints over in rCells.
    OString writeObjectContent(SdrObject& rObject, const ScAddress& rAnchor, ScRange& rCells);
    /// Give a held block the version of a change to its objects, so it is sent again.
    static void bumpBlock(SheetState& rState, const BlockKey& rKey, sal_uInt64 nVersion);
    void resolveSheet(SheetState& rState);
    OString writeTextEdits(SCTAB nTab);
    /// The shape whose text the view edits on the sheet, or nullptr.
    static const SdrObject* shapeTextEditOn(ScTabViewShell& rViewShell, SCTAB nTab);
    /// Whether a view edits the text of a shape on the sheet.
    bool anyShapeTextEditOn(SCTAB nTab) const;

    /// Write a block with the parts and objects changed after nFrom, which is all a client holding
    /// the block at nFrom or later lacks. Add the text styles those parts name to rTextStyles.
    static void writeBlockEntry(tools::JsonWriter& rWriter, const SheetState& rState,
                                const BlockKey& rKey, const BlockState& rBlock, sal_uInt64 nFrom,
                                std::set<sal_Int32>& rTextStyles);
    static void writeObjectEntry(tools::JsonWriter& rWriter, sal_uInt64 nId,
                                 const EntryState& rEntry);

    ScDocShell& mrDocShell;
    drawinglayer::VectorResourceCache maResources;
    /// The font attributes of the cell text of every sheet, which the text names by id.
    drawinglayer::TextStyleTable maTextStyles;
    /// Listens to written charts, which change with their data without a drawing layer hint.
    rtl::Reference<ChartModifyListener> mxChartListener;
    /// Hears of cells with new results on each served sheet, which come without a paint.
    std::map<SCTAB, std::unique_ptr<SheetListener>> maSheetListeners;
    /// The sheets with cells changed since the views were last told.
    std::set<SCTAB> maChangedTabs;
    /// The state of each sheet, for each mode it was asked in.
    std::map<std::pair<SCTAB, sal_Int32>, SheetState> maSheets;
    /// An answer or a push is being written from a sheet state.
    bool mbWriting = false;
    /// The sheets have to be reset once the write in progress is done.
    bool mbResetSheetsPending = false;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
