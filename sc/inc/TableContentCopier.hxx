/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "address.hxx"
#include "global.hxx"

class ScDocument;
class ScTable;
class ScMarkData;

namespace sc
{
struct RefUpdateInsertTabContext;

/** Copies the content of one table (sheet) to another within the same document. */
class TableContentCopier
{
    ScDocument& mrDoc;
    const SCTAB mnSourceTabNo;
    const SCTAB mnTargetTabNo;
    ScTable* const mpSourceTab;
    ScTable* const mpTargetTab;

public:
    TableContentCopier(ScDocument& rDoc, SCTAB nSourceTabNo, SCTAB nTargetTabNo);

    ~TableContentCopier();

    void performCopy(const ScMarkData* pOnlyMarked, ScCloneFlags nCloneFlags,
                     SCTAB nPreviousSourceTabNo = -1);
    void updateReferencesAfterTabInsertion(RefUpdateInsertTabContext& rContext);
    void recompileTargetFormulas();
};

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
