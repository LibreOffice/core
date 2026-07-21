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
    /// of nDefaultViewTab on the views that are not currently showing them. The
    /// normal paint path only invalidates the tab a view currently displays, so
    /// content synced into the other parts would otherwise leave their tiles
    /// stale until the user switches to them.
    static void invalidateSheetViewTiles(ScDocShell& rDocShell, SCTAB nDefaultViewTab);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
