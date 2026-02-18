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

class ScMarkData;
class ScAddress;
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

    /** Convert address from a sheet view to the address in default view, take sorting into account. */
    ScAddress convertAddress(ScAddress const& rAddress);

    /** Convert a mark from a sheet view to the mark in default view, take sorting into account. */
    ScMarkData convertMark(ScMarkData const& rMarkData);

    /** Synchronizes the sheet views and the default view */
    void syncSheetViews();

public:
    Operation(OperationType eType, bool bRecord, bool bApi);

    bool run();

    virtual bool runImplementation() = 0;
};

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
