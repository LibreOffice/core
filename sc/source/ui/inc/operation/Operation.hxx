/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <operation/OperationType.hxx>
#include <types.hxx>

class ScMarkData;
class ScAddress;
class ScRange;
class ScViewData;

namespace sc
{
/** Operation is one atomic coarse change to the document model that can be run from UI or API
 *
 * An operation is a minimal change that is undoable and redoable.
 * It should provide and handle:
 * - document modification flag
 * - undo/redo if enabled
 * - sheet protections
 * - sheet view sync and if it can be performed
 * - API / UI code paths
 * - invalidation
 * - logging
 * - other common tasks
 *
 * Each operation should have an OperationType.
 */
class Operation
{
protected:
    OperationType meType = OperationType::Unknown;
    bool mbApi : 1 = false;
    bool mbRecord : 1 = false;
    ScViewData* mpViewData;

    bool checkSheetViewProtection();

    /** Convert a tab from a sheet view to the tab in the default view. */
    SCTAB convertTab(SCTAB nTab);

    /** Convert address from a sheet view to the address in default view, take sorting into account. */
    ScAddress convertAddress(ScAddress const& rAddress);

    /** Convert a range from a sheet view to the range in default view, take sorting into account. */
    ScRange convertRange(ScRange const& rRange);

    /** Convert a mark from a sheet view to the mark in default view, take sorting into account. */
    ScMarkData convertMark(ScMarkData const& rMarkData);

    /** Synchronizes the sheet views and the default view */
    void syncSheetViews();

    /** Check if the input is on a sheet view tab */
    bool isInputOnSheetView() const;

    /** Check if a range is on a sheet view tab and intersects with any autofilter */
    bool isInputOnSheetViewAutoFilter(ScRange const& rRange) const;

    /** Check if we can run the operation or not */
    virtual bool canRunTheOperation() const { return true; }

    /** Operation implementation */
    virtual bool runImplementation() = 0;

public:
    Operation(OperationType eType, bool bRecord, bool bApi);

    /** Run the operation */
    bool run();
};

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
