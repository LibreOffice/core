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

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

class ScDocument;

namespace sc {

struct RefUpdateContext
{
    typedef boost::unordered_set<sal_uInt16> NameIndicesType;
    typedef boost::unordered_map<SCTAB, NameIndicesType> UpdatedNamesType;

    ScDocument& mrDoc;

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

    /** All named expressions that have been updated during this reference
     *  update run. */
    UpdatedNamesType maUpdatedNames;

    RefUpdateContext(ScDocument& rDoc);

    bool isInserted() const;
    bool isDeleted() const;

    void setUpdatedName(SCTAB nTab, sal_uInt16 nIndex);
    bool isNameUpdated(SCTAB nTab, sal_uInt16 nIndex) const;
};

struct RefUpdateResult
{
    bool mbValueChanged;
    bool mbReferenceModified;
    bool mbNameModified;

    RefUpdateResult();
    RefUpdateResult(const RefUpdateResult& r);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
