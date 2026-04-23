/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <SheetView.hxx>
#include <SheetViewManager.hxx>
#include <rangelst.hxx>

class ScDocShell;

namespace sc
{
/** Mixin for undo/redo of sheet view sort data and auto-filter DB range.
 *
 * Captures before/after state of SheetView and SheetViewManager sort data,
 * and restores them on undo/redo. Used by any undo action that modifies
 * sort data (sorting, insert/delete cells, auto-filter expansion,...)
 */
class UndoSheetViewSortData
{
    SCTAB mnDefaultViewTab = -1;

    // For operations on a sheet view tab
    SheetViewID mnSheetViewID = -1;
    std::shared_ptr<SheetViewSortData> mpSheetViewSortDataBefore;
    std::shared_ptr<SheetViewSortData> mpSheetViewSortDataAfter;

    // For operations on a default view tab
    std::shared_ptr<DefaultViewSortData> mpDefaultViewSortDataBefore;
    std::shared_ptr<DefaultViewSortData> mpDefaultViewSortDataAfter;

    // Auto-filter DB range before/after
    std::optional<ScRange> moAutoFilterRangeBefore;
    std::optional<ScRange> moAutoFilterRangeAfter;

public:
    /** Set context for operations on a sheet view tab. */
    void setSheetViewContext(SCTAB nDefaultViewTab, SheetViewID nSheetViewID,
                             std::shared_ptr<SheetViewSortData> pBefore,
                             std::shared_ptr<SheetViewSortData> pAfter);

    /** Set context for operations on a default view tab. */
    void setDefaultViewContext(SCTAB nDefaultViewTab, std::shared_ptr<DefaultViewSortData> pBefore,
                               std::shared_ptr<DefaultViewSortData> pAfter);

    /** Set auto-filter DB range before/after for undo/redo. */
    void setAutoFilterRange(ScRange const& rBefore, ScRange const& rAfter);

    /** Restore sort data for undo (before state) or redo (after state). */
    void restore(ScDocShell& rDocShell, bool bUndo) const;

    /** True if any sort data was captured. */
    bool hasData() const { return mnDefaultViewTab >= 0; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
