/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Operation.hxx"
#include <cellmergeoption.hxx>

class ScDocShell;

namespace sc
{
/** Merges a range of cells into one. */
class MergeCellsOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScCellMergeOption maOption;
    bool mbContents;
    bool mbEmptyMergedCells;

    bool runImplementation() override;

public:
    MergeCellsOperation(ScDocShell& rDocShell, const ScCellMergeOption& rOption, bool bContents,
                        bool bRecord, bool bApi, bool bEmptyMergedCells = false);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
