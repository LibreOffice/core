/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <refupdatecontext.hxx>
#include <algorithm>

namespace sc {

void UpdatedRangeNames::setUpdatedName(SCTAB nTab, sal_uInt16 nIndex)
{
    // Map anything <-1 to global names. Unless we really want to come up with
    // some classification there..
    if (nTab < -1)
        nTab = -1;

    UpdatedNamesType::iterator it = maUpdatedNames.find(nTab);
    if (it == maUpdatedNames.end())
    {
        // Insert a new container for this sheet index.
        NameIndicesType aIndices;
        std::pair<UpdatedNamesType::iterator,bool> r =
            maUpdatedNames.emplace( nTab, aIndices);

        if (!r.second)
            // Insertion failed for whatever reason.
            return;

        it = r.first;
    }

    NameIndicesType& rIndices = it->second;
    rIndices.insert(nIndex);
}

bool UpdatedRangeNames::isNameUpdated(SCTAB nTab, sal_uInt16 nIndex) const
{
    UpdatedNamesType::const_iterator it = maUpdatedNames.find(nTab);
    if (it == maUpdatedNames.end())
        return false;

    const NameIndicesType& rIndices = it->second;
    return rIndices.count(nIndex) > 0;
}

UpdatedRangeNames::NameIndicesType UpdatedRangeNames::getUpdatedNames(SCTAB nTab) const
{
    UpdatedNamesType::const_iterator it = maUpdatedNames.find(nTab);
    if (it == maUpdatedNames.end())
        return NameIndicesType();
    return it->second;
}

bool UpdatedRangeNames::isEmpty(SCTAB nTab) const
{
    UpdatedNamesType::const_iterator it = maUpdatedNames.find(nTab);
    return it == maUpdatedNames.end();
}


RefUpdateContext::RefUpdateContext(ScDocument& rDoc) :
    mrDoc(rDoc), meMode(URM_INSDEL), mnColDelta(0), mnRowDelta(0), mnTabDelta(0) {}

bool RefUpdateContext::isInserted() const
{
    return (meMode == URM_INSDEL) && (mnColDelta > 0 || mnRowDelta > 0 || mnTabDelta > 0);
}

bool RefUpdateContext::isDeleted() const
{
    return (meMode == URM_INSDEL) && (mnColDelta < 0 || mnRowDelta < 0 || mnTabDelta < 0);
}

RefUpdateResult::RefUpdateResult() : mbValueChanged(false), mbReferenceModified(false), mbNameModified(false) {}

RefUpdateInsertTabContext::RefUpdateInsertTabContext(ScDocument& rDoc, SCTAB nInsertPos, SCTAB nSheets) :
    mrDoc(rDoc), mnInsertPos(nInsertPos), mnSheets(nSheets) {}

RefUpdateDeleteTabContext::RefUpdateDeleteTabContext(ScDocument& rDoc, SCTAB nDeletePos, SCTAB nSheets) :
    mrDoc(rDoc), mnDeletePos(nDeletePos), mnSheets(nSheets) {}

RefUpdateMoveTabContext::RefUpdateMoveTabContext(ScDocument& rDoc, SCTAB nOldPos, SCTAB nNewPos) :
    mrDoc(rDoc), mnOldPos(nOldPos), mnNewPos(nNewPos) {}

SCTAB RefUpdateMoveTabContext::getNewTab(SCTAB nOldTab) const
{
    // Sheets below the lower bound or above the uppper bound will not change.
    SCTAB nLowerBound = std::min(mnOldPos, mnNewPos);
    SCTAB nUpperBound = std::max(mnOldPos, mnNewPos);

    if (nOldTab < nLowerBound || nUpperBound < nOldTab)
        // Outside the boundary. Nothing to adjust.
        return nOldTab;

    if (nOldTab == mnOldPos)
        return mnNewPos;

    // It's somewhere in between.
    if (mnOldPos < mnNewPos)
    {
        // Moving a sheet to the right. The rest of the sheets shifts to the left.
        return nOldTab - 1;
    }

    // Moving a sheet to the left. The rest of the sheets shifts to the right.
    return nOldTab + 1;
}

SetFormulaDirtyContext::SetFormulaDirtyContext() :
    mnTabDeletedStart(-1), mnTabDeletedEnd(-1), mbClearTabDeletedFlag(false) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
