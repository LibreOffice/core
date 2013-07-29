/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "refupdatecontext.hxx"

namespace sc {

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

void RefUpdateContext::setUpdatedName(SCTAB nTab, sal_uInt16 nIndex)
{
    UpdatedNamesType::iterator it = maUpdatedNames.find(nTab);
    if (it == maUpdatedNames.end())
    {
        // Insert a new container for this sheet index.
        NameIndicesType aIndices;
        std::pair<UpdatedNamesType::iterator,bool> r =
            maUpdatedNames.insert(UpdatedNamesType::value_type(nTab, aIndices));

        if (!r.second)
            // Insertion failed for whatever reason.
            return;

        it = r.first;
    }

    NameIndicesType& rIndices = it->second;
    rIndices.insert(nIndex);
}

bool RefUpdateContext::isNameUpdated(SCTAB nTab, sal_uInt16 nIndex) const
{
    UpdatedNamesType::const_iterator it = maUpdatedNames.find(nTab);
    if (it == maUpdatedNames.end())
        return false;

    const NameIndicesType& rIndices = it->second;
    return rIndices.count(nIndex) > 0;
}

RefUpdateResult::RefUpdateResult() : mbValueChanged(false), mbReferenceModified(false), mbNameModified(false) {}
RefUpdateResult::RefUpdateResult(const RefUpdateResult& r) :
    mbValueChanged(r.mbValueChanged),
    mbReferenceModified(r.mbReferenceModified),
    mbNameModified(r.mbNameModified) {}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
