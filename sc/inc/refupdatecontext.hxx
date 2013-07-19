/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_REFUPDATECONTEXT_HXX
#define SC_REFUPDATECONTEXT_HXX

#include "global.hxx"
#include "address.hxx"

namespace sc {

struct RefUpdateContext
{
    /**
     * update mode - insert/delete, copy, or move. The reorder mode (which
     * corresponds with the reordering of sheets) is not used with this
     * context.
     */
    UpdateRefMode meMode;

    /**
     * Range of cells that are about to be moved for insert/delete/move modes.
     * For copy mode, it's the destination range of cells that are about to be
     * pasted.
     */
    ScRange maRange;

    /** Amount and direction of movement in the column direction. */
    SCCOL mnColDelta;
    /** Amount and direction of movement in the row direction. */
    SCROW mnRowDelta;
    /** Amount and direction of movement in the sheet direction. */
    SCTAB mnTabDelta;

    RefUpdateContext();

    bool isInserted() const;
    bool isDeleted() const;
};

struct RefUpdateResult
{
    bool mbValueChanged;
    bool mbRangeSizeModified;

    RefUpdateResult();
    RefUpdateResult(const RefUpdateResult& r);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
