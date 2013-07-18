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

RefUpdateContext::RefUpdateContext() :
    meMode(URM_INSDEL), mnColDelta(0), mnRowDelta(0), mnTabDelta(0) {}

bool RefUpdateContext::hasDelta() const
{
    return (mnColDelta > 0 || mnRowDelta > 0 || mnTabDelta > 0);
}

RefUpdateResult::RefUpdateResult() : mbValueChanged(false), mbRangeSizeModified(false) {}
RefUpdateResult::RefUpdateResult(const RefUpdateResult& r) :
    mbValueChanged(r.mbValueChanged), mbRangeSizeModified(r.mbRangeSizeModified) {}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
