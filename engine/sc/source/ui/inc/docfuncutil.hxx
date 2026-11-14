/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "undobase.hxx"

#include <memory>

class ScAddress;
class ScDocFunc;
class ScDocShell;
class ScMarkData;
class ScRange;
enum class InsertDeleteFlags : sal_Int32;
namespace tools { class Rectangle; }

namespace sc {

class DocFuncUtil
{
public:

    static bool hasProtectedTab( const ScDocument& rDoc, const ScMarkData& rMark );

    /// If the cell at rPosition is the master of a multi-cell matrix formula,
    /// delete the master cell so the whole matrix (master plus reference cells) 
    //  is removed.
    /// Returns true if a demolition happened. Used before single-cell
    /// writes so the new value doesn't leave reference cells orphaned.
    static bool demolishMatrixMaster(
        ScDocFunc& rDocFunc, const ScDocument& rDoc, const ScAddress& rPosition, bool bApi);

    static ScDocumentUniquePtr createDeleteContentsUndoDoc(
        ScDocument& rDoc, const ScMarkData& rMark, const ScRange& rRange,
        InsertDeleteFlags nFlags, bool bOnlyMarked );

    static void addDeleteContentsUndo(
        SfxUndoManager* pUndoMgr, ScDocShell& rDocSh, const ScMarkData& rMark,
        const ScRange& rRange, ScDocumentUniquePtr&& pUndoDoc, InsertDeleteFlags nFlags,
        const std::shared_ptr<ScSimpleUndo::DataSpansType>& pSpans,
        bool bMulti, bool bDrawUndo,
        std::vector<ScAddress> const& rRestoreExpandedMatrices);

    static std::shared_ptr<ScSimpleUndo::DataSpansType> getNonEmptyCellSpans(
        const ScDocument& rDoc, const ScMarkData& rMark, const ScRange& rRange );

    /// Invalidate the tiles of the base sheet and every sheet view holder table
    /// of nDefaultViewTab. The normal paint path only invalidates the tab a view
    /// currently displays, so content synced into the other parts would
    /// otherwise leave their tiles stale until the user switches to them. When
    /// bIncludeViewShownPart is true the invalidation also reaches a view that
    /// is currently showing one of these parts; pass true from a caller that did
    /// not repaint the part on the view showing it (for example an undo that
    /// repaints only the base sheet), so that view is refreshed too.
    static void invalidateSheetViewTiles(ScDocShell& rDocShell, SCTAB nDefaultViewTab,
                                         bool bIncludeViewShownPart = false);

    /// Copy the content of the table nTab to its sheet views and invalidate their tiles on
    /// every view, including the views that show them. A sheet view holder table has no sheet
    /// views of its own, so nothing changes for one.
    static void syncSheetViewsOfTable(ScDocShell& rDocShell, SCTAB nTab);

    /// True when rRange is on a sheet view holder table and overlaps an auto-filter range of
    /// that table.
    static bool isRangeOnSheetViewAutoFilter(ScDocument& rDocument, const ScRange& rRange);

    /// True when rRange is on a sheet view holder table and overlaps rows that the sheet view
    /// can order or hide differently from its default view: an auto-filter range of the holder
    /// table, the range the sheet view is sorted on, or a range the default view was sorted on
    /// after the sheet view was created. Every other row of the holder table is the same row of
    /// the default view table.
    static bool isRangeOnSheetViewOwnRows(ScDocument& rDocument, const ScRange& rRange);

    /// A rectangle larger than any sheet, so it covers every tile of a part regardless of
    /// that part's actual size. Pass it to KitHelper::notifyInvalidationAllViews to invalidate
    /// a whole part by its index rather than by a specific area.
    static tools::Rectangle wholePartInvalidationRectangle();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
