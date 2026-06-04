/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Operation.hxx"
#include <cellmergeoption.hxx>

class ScDocShell;
class ScDocFunc;
class ScUndoRemoveMerge;

namespace sc
{
/** Removes a cell-merge attribute from a range of cells.
 *
 *  When pUndoRemoveMerge is non-null, the caller is collecting several
 *  unmerges into one undo step and this operation just appends its
 *  option to it. Otherwise the operation creates and registers its own
 *  ScUndoRemoveMerge.
 */
class UnmergeCellsOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScCellMergeOption maOption;
    ScUndoRemoveMerge* mpUndoRemoveMerge;

    bool runImplementation() override;

public:
    UnmergeCellsOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                          ScCellMergeOption const& rOption, bool bRecord,
                          ScUndoRemoveMerge* pUndoRemoveMerge);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
